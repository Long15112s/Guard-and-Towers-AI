#include "TT_PVSAI.hpp"
#include "AIRegistry.hpp"

// Registrierung im Factory-Pattern:
REGISTER_AI(ai::TT_PVSAI)

#include <algorithm>
#include <iostream>
#include <cmath>

namespace ai {

//-----------------------------------------------------------------------------
// Statische Member initialisieren
//-----------------------------------------------------------------------------
bool     TT_PVSAI::_zobristInited = false;
uint64_t TT_PVSAI::_zTable[49][4]  = {{0}};
uint64_t TT_PVSAI::_zSide          = 0;

//-----------------------------------------------------------------------------
// Konstruktor
//-----------------------------------------------------------------------------
TT_PVSAI::TT_PVSAI(int maxDepth, int maxTimeMs)
  : _maxDepth(maxDepth),
    _maxTimeMs(maxTimeMs),
    _nodeCount(0),
    _startTimeMs(0)
{
    // Zobrist-Tabellen nur einmal initialisieren (deterministischer Seed)
    if (!_zobristInited) {
        initZobrist();
        _zobristInited = true;
    }
    // Killer-Moves und History-Caches leeren
    for (int d = 0; d < MAX_DEPTH_SUPPORTED; ++d) {
        _killerMoves[d][0] = ::Move{};
        _killerMoves[d][1] = ::Move{};
    }
    std::fill(_historyScore, _historyScore + MAX_MOVES, 0);
}

//-----------------------------------------------------------------------------
// Zobrist-Initialisierung
//-----------------------------------------------------------------------------
void TT_PVSAI::initZobrist() {
    std::mt19937_64 rng(0xDEADBEEF);
    std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());

    for (int sq = 0; sq < 49; ++sq) {
        for (int pt = 0; pt < 4; ++pt) {
            _zTable[sq][pt] = dist(rng);
        }
    }
    _zSide = dist(rng);
}

//-----------------------------------------------------------------------------
// Berechne Zobrist-Hash für eine Stellung
//-----------------------------------------------------------------------------
uint64_t TT_PVSAI::computeHash(const ::Board& board, ::Color side) const {
    uint64_t h = 0ULL;
    for (int sq = 0; sq < 49; ++sq) {
        if      (board.hasGuard(::Color::Red,   sq)) h ^= _zTable[sq][0];
        else if (board.hasGuard(::Color::Blue,  sq)) h ^= _zTable[sq][1];
        else if (board.hasStack(::Color::Red,   sq)) h ^= _zTable[sq][2];
        else if (board.hasStack(::Color::Blue,  sq)) h ^= _zTable[sq][3];
    }
    if (side == ::Color::Blue) {
        h ^= _zSide;
    }
    return h;
}

//-----------------------------------------------------------------------------
// Timeout prüfen
//-----------------------------------------------------------------------------
bool TT_PVSAI::isTimeout() const {
    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::high_resolution_clock::now().time_since_epoch())
                       .count();
    return (now - _startTimeMs) > static_cast<uint64_t>(_maxTimeMs);
}

//-----------------------------------------------------------------------------
// Öffentliche Schnittstelle: chooseMove
//  → Iteratives Deepening von Tiefe = 1 bis _maxDepth
//-----------------------------------------------------------------------------
::Move TT_PVSAI::chooseMove(const ::Board& board, ::Color side) {
    std::vector<::Move> moves;
    MoveGenerator::generate(board, moves);
    if (moves.empty()) return ::Move{};

    ::Move finalBest = moves[0];
    int lastDepth = 0;

    // Startzeit merken (ms)
    _startTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::high_resolution_clock::now().time_since_epoch())
                       .count();

    // Iterative Deepening
    for (int depth = 1; depth <= _maxDepth; ++depth) {
        if (isTimeout()) {
            std::cout << "[TT_PVSAI] Timeout nach Tiefe " << (depth - 1) << "\n";
            break;
        }

        // Vor jedem neuen Iterationsschritt: TT leeren,
        // NodeCount zurücksetzen, KillerMoves und History-Score leeren
        _tt.clear();
        _nodeCount = 0;
        for (int d = 0; d < MAX_DEPTH_SUPPORTED; ++d) {
            _killerMoves[d][0] = ::Move{};
            _killerMoves[d][1] = ::Move{};
        }
        std::fill(_historyScore, _historyScore + MAX_MOVES, 0);

        auto t0 = std::chrono::high_resolution_clock::now();
        ::Move bestAtDepth = rootSearch(board, depth, side);
        auto t1 = std::chrono::high_resolution_clock::now();

        // Fließkomma-Millisekunden
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        std::cout << "[TT_PVSAI] Tiefe=" << depth
                  << "  best="  << MoveGenerator::toString(bestAtDepth)
                  << "  nodes=" << _nodeCount
                  << "  Zeit="  << ms << " ms\n";

        finalBest = bestAtDepth;
        lastDepth = depth;
    }

    std::cout << "[TT_PVSAI] letzte fertige Tiefe=" << lastDepth << "\n";
    return finalBest;
}

//-----------------------------------------------------------------------------
// Root-Funktion: probiere alle Züge mit PVS+TT auf depth-1
//-----------------------------------------------------------------------------
::Move TT_PVSAI::rootSearch(const ::Board& board, int depth, ::Color side) {
    std::vector<::Move> moves;
    MoveGenerator::generate(board, moves);
    if (moves.empty()) return ::Move{};

    ::Move bestMove = moves[0];
    int bestScore = std::numeric_limits<int>::min();

    int alpha = std::numeric_limits<int>::min();
    int beta  = std::numeric_limits<int>::max();

    // 1) Prüfe TT-Eintrag in der Wurzelstellung
    uint64_t rootKey = computeHash(board, side);
    TTEntry* rootTT = nullptr;
    auto itRoot = _tt.find(rootKey);
    if (itRoot != _tt.end() && itRoot->second.depth >= depth) {
        rootTT = &(itRoot->second);
    }

    // 2) Move-Ordering: TT-bestMove ans Front, dann Killer, Captures, History, Rest
    orderMoves(board, moves, rootTT, depth, side);

    bool firstChild = true;
    for (auto& m : moves) {
        if (isTimeout()) break;

        ::Board tmp = board;
        tmp.makeMove(m);

        int score;
        if (firstChild) {
            // Erstes Kind: volles Fenster
            score = pvsWithTT(tmp, depth - 1, alpha, beta, side, false);
            firstChild = false;
        } else {
            // Null-Fenster-Test mit [alpha, alpha+1]
            int scoreTest = pvsWithTT(tmp, depth - 1, alpha, alpha + 1, side, false);
            if (scoreTest > alpha && scoreTest < beta) {
                // Re-Suche, falls nötig
                score = pvsWithTT(tmp, depth - 1, alpha, beta, side, false);
            } else {
                score = scoreTest;
            }
        }

        if (score > bestScore) {
            bestScore = score;
            bestMove  = m;
        }
        alpha = std::max(alpha, bestScore);
    }

    return bestMove;
}

//-----------------------------------------------------------------------------
// PVS-Suche kombiniert mit Transposition Table (TT), Killer, History
//-----------------------------------------------------------------------------
int TT_PVSAI::pvsWithTT(::Board& board,
                        int depth,
                        int alpha,
                        int beta,
                        ::Color side,
                        bool maximizing)
{
    // 1) Timeout-Check
    if (isTimeout()) {
        ++_nodeCount;
        return eval(board, side);
    }

    // 2) Zobrist-Hash + TT-Lookup
    uint64_t key = computeHash(board, side);
    auto it = _tt.find(key);
    if (it != _tt.end()) {
        TTEntry& entry = it->second;
        if (entry.depth >= depth) {
            if (entry.flag == TTFlag::EXACT) {
                return entry.value;
            }
            if (entry.flag == TTFlag::LOWERBOUND && entry.value > alpha) {
                alpha = entry.value;
            } else if (entry.flag == TTFlag::UPPERBOUND && entry.value < beta) {
                beta = entry.value;
            }
            if (alpha >= beta) {
                return entry.value;
            }
        }
    }

    // 3) Terminal: depth == 0 oder Spielende
    if (depth == 0 || board.isGameOver()) {
        ++_nodeCount;
        int val = eval(board, side);

        // Speichere als EXACT im TT
        TTEntry leafEntry;
        leafEntry.key      = key;
        leafEntry.depth    = depth;
        leafEntry.value    = val;
        leafEntry.flag     = TTFlag::EXACT;
        leafEntry.bestMove = ::Move{};
        _tt[key] = leafEntry;
        return val;
    }

    // 4) Generiere alle legalen Züge
    std::vector<::Move> moves;
    MoveGenerator::generate(board, moves);
    if (moves.empty()) {
        ++_nodeCount;
        int val = eval(board, side);

        // Speichere im TT
        TTEntry e2;
        e2.key      = key;
        e2.depth    = depth;
        e2.value    = val;
        e2.flag     = TTFlag::EXACT;
        e2.bestMove = ::Move{};
        _tt[key] = e2;
        return val;
    }

    // 5) Move-Ordering vor Schleife:
    //    • Falls TT-Eintrag existiert → entry.bestMove zuerst
    //    • Dann KillerMoves für diese Tiefe
    //    • Dann Captures (MVV-LVA)
    //    • Dann History-Züge
    //    • Dann Rest
    TTEntry* hitEntry = nullptr;
    if (it != _tt.end()) {
        hitEntry = &it->second;
    }
    orderMoves(board, moves, hitEntry, depth, side);

    // 6) PVS-Iteration über alle Kinder
    int originalAlpha = alpha;
    ::Move bestLocalMove{};
    int value;

    if (maximizing) {
        value = std::numeric_limits<int>::min();
        bool firstChild = true;
        for (auto& m : moves) {
            if (isTimeout()) break;

            ::Board tmp = board;
            tmp.makeMove(m);

            int score;
            if (firstChild) {
                // Erstes Kind: volles Fenster
                score = pvsWithTT(tmp, depth - 1, alpha, beta, side, false);
                firstChild = false;
            } else {
                // Null-Fenster-Test
                score = pvsWithTT(tmp, depth - 1, alpha, alpha + 1, side, false);
                if (score > alpha && score < beta) {
                    // Re-Suche, falls nötig
                    score = pvsWithTT(tmp, depth - 1, alpha, beta, side, false);
                }
            }

            if (score > value) {
                value = score;
                bestLocalMove = m;
            }

            // Beta-Cutoff (maximizing)
            if (value > alpha) {
                alpha = value;
            }
            if (alpha >= beta) {
                // Killer-Heuristik: m ist Killer-Move auf dieser Tiefe
                if (depth < MAX_DEPTH_SUPPORTED) {
                    _killerMoves[depth][1] = _killerMoves[depth][0];
                    _killerMoves[depth][0] = m;
                }
                // History-Update: belohne diesen Zug an dieser Tiefe
                {
                    int idx = (int)m.from * 49 * MAX_DEPTH_SUPPORTED
                              + (int)m.to   * MAX_DEPTH_SUPPORTED
                              + (depth - 1);
                    _historyScore[idx] += depth * depth;
                }
                break;
            }
        }
    } else {
        // Minimizing-Seite (negamax-aspekt umgekehrt)
        value = std::numeric_limits<int>::max();
        bool firstChild = true;
        for (auto& m : moves) {
            if (isTimeout()) break;

            ::Board tmp = board;
            tmp.makeMove(m);

            int score;
            if (firstChild) {
                score = pvsWithTT(tmp, depth - 1, alpha, beta, side, true);
                firstChild = false;
            } else {
                score = pvsWithTT(tmp, depth - 1, beta - 1, beta, side, true);
                if (score > alpha && score < beta) {
                    score = pvsWithTT(tmp, depth - 1, alpha, beta, side, true);
                }
            }

            if (score < value) {
                value = score;
                bestLocalMove = m;
            }

            // Alpha-Cutoff (minimizing)
            if (value < beta) {
                beta = value;
            }
            if (beta <= alpha) {
                // Killer-Heuristik: m ist Killer-Move
                if (depth < MAX_DEPTH_SUPPORTED) {
                    _killerMoves[depth][1] = _killerMoves[depth][0];
                    _killerMoves[depth][0] = m;
                }
                // History-Update: belohne Weg, der zu Cutoff geführt hat
                {
                    int idx = (int)m.from * 49 * MAX_DEPTH_SUPPORTED
                              + (int)m.to   * MAX_DEPTH_SUPPORTED
                              + (depth - 1);
                    _historyScore[idx] += depth * depth;
                }
                break;
            }
        }
    }

    ++_nodeCount;

    // 7) Speichere in TT
    TTEntry newEntry;
    newEntry.key      = key;
    newEntry.depth    = depth;
    newEntry.value    = value;
    newEntry.bestMove = bestLocalMove;

    if (value <= originalAlpha) {
        newEntry.flag = TTFlag::UPPERBOUND;  
    } else if (value >= beta) {
        newEntry.flag = TTFlag::LOWERBOUND;
    } else {
        newEntry.flag = TTFlag::EXACT;
    }
    _tt[key] = newEntry;

    return value;
}

//-----------------------------------------------------------------------------
// Move-Ordering-Funktion
//   1) TT-bestMove ans Front
//   2) Killer-Moves (max. 2)
//   3) Schlagende Züge (MVV-LVA sortiert absteigend)
//   4) History-Heuristik sortiert (ruhige) Züge absteigend
//   5) Alle übrigen Züge
//-----------------------------------------------------------------------------
void TT_PVSAI::orderMoves(const ::Board& board,
                          std::vector<::Move>& moves,
                          const TTEntry* ttHit,
                          int currentDepth,
                          ::Color side) const
{
    // 1. Lambda für Move-Gleichheit (von MoveGenerator)
    auto isSameMove = [&](const ::Move &a, const ::Move &b) {
        return (a.from == b.from && a.to == b.to && a.splitCount == b.splitCount);
    };

    // 2. Bestimme, ob ein Move ein Capture ist, und berechne MVV-LVA-Score:
    auto isCapture = [&](const ::Move &m_in) {
        ::Color victimColor = (side == ::Color::Red ? ::Color::Blue : ::Color::Red);
        // Ein Capture liegt vor, wenn am Zielfeld eine gegnerische Guard oder Stack steht:
        return board.hasGuard(victimColor, m_in.to)
            || board.hasStack(victimColor, m_in.to);
    };
    auto mvvLvaScore = [&](const ::Move &m_in) {
        if (!isCapture(m_in)) return -1000000;
        ::Color victimColor = (side == ::Color::Red ? ::Color::Blue : ::Color::Red);
        // Opfer-Wert: Guard=100, Stack=1
        int victimValue = board.hasGuard(victimColor, m_in.to) ? 100 : 1;
        // Aggressor-Wert: splitCount==0 ⇒ Guard=100, else Stack=1
        int aggressorValue = (m_in.splitCount == 0 ? 100 : 1);
        // Score so hoch wie möglich, wenn Victim groß und Aggressor klein:
        return victimValue * 1000 - aggressorValue;
    };

    // 3. History-Score-Indexfunktion
    auto historyIndex = [&](const ::Move &m_in) {
        return (int)m_in.from * 49 * MAX_DEPTH_SUPPORTED
             + (int)m_in.to   * MAX_DEPTH_SUPPORTED
             + (currentDepth - 1);
    };

    // 4. Wir bauen mehrere Buckets:
    std::vector<::Move> bucketTT;
    std::vector<::Move> bucketKiller;
    std::vector<::Move> bucketCapture;
    std::vector<::Move> bucketHistory;
    std::vector<::Move> bucketRest;

    // 5. Spalte die Moves auf:
    for (auto& m : moves) {
        // a) TT-BestMove?
        if (ttHit && isSameMove(m, ttHit->bestMove)) {
            bucketTT.push_back(m);
            continue;
        }
        // b) Killer-Moves?
        bool isKiller = false;
        if (currentDepth < MAX_DEPTH_SUPPORTED) {
            for (int k = 0; k < 2; ++k) {
                if (isSameMove(m, _killerMoves[currentDepth][k])) {
                    bucketKiller.push_back(m);
                    isKiller = true;
                    break;
                }
            }
            if (isKiller) continue;
        }
        // c) Capture?
        if (isCapture(m)) {
            bucketCapture.push_back(m);
            continue;
        }
        // d) History-Zug (wenn History-Score > 0)
        int idx = historyIndex(m);
        if (_historyScore[idx] > 0) {
            bucketHistory.push_back(m);
            continue;
        }
        // e) Sonst normaler Zug
        bucketRest.push_back(m);
    }

    // 6. Sortiere bucketCapture nach MVV-LVA absteigend:
    std::sort(bucketCapture.begin(), bucketCapture.end(),
        [&](const ::Move &a, const ::Move &b) {
            return mvvLvaScore(a) > mvvLvaScore(b);
        }
    );

    // 7. Sortiere bucketHistory nach historischer Score absteigend:
    std::sort(bucketHistory.begin(), bucketHistory.end(),
        [&](const ::Move &a, const ::Move &b) {
            return _historyScore[historyIndex(a)]
                 > _historyScore[historyIndex(b)];
        }
    );

    // 8. Final zusammenbauen:
    std::vector<::Move> ordered;
    ordered.reserve(moves.size());
    for (auto &m : bucketTT)      ordered.push_back(m);
    for (auto &m : bucketKiller)  ordered.push_back(m);
    for (auto &m : bucketCapture) ordered.push_back(m);
    for (auto &m : bucketHistory) ordered.push_back(m);
    for (auto &m : bucketRest)    ordered.push_back(m);

    moves.swap(ordered);
}

//-----------------------------------------------------------------------------
// Evaluation (exakt aus AlphaBetaAI / PVSAI) 
//-----------------------------------------------------------------------------
int TT_PVSAI::eval(const ::Board& board, ::Color side) const {
    ::Color enemy = (side == ::Color::Red ? ::Color::Blue : ::Color::Red);

    // 0) Guaranteed-Win-Check (freier Weg zum Ziel)
    int myG = -1;
    for (int sq = 0; sq < 49; ++sq) {
        if (board.hasGuard(side, sq)) myG = sq;
    }
    int goal = (side == ::Color::Red ? RED_GOAL_SQ : BLUE_GOAL_SQ);
    int dMy  = std::abs(myG/7 - goal/7) + std::abs(myG%7 - goal%7);

    int dOpp = std::numeric_limits<int>::max();
    for (int sq = 0; sq < 49; ++sq) {
        if (board.hasGuard(enemy, sq) || board.hasStack(enemy, sq)) {
            dOpp = std::min(dOpp, std::abs(sq/7 - goal/7) + std::abs(sq%7 - goal%7));
        }
    }
    if (dMy < dOpp) {
        return +10000000 - dMy;
    }

    // 1) Terminal (Spielende)
    if (board.isGameOver()) {
        return (board.winner() == side ? +1000000 : -1000000);
    }

    int score = 0;

    // 2) Material
    score += 5000;
    int ownT = 0, oppT = 0;
    for (int sq = 0; sq < 49; ++sq) {
        if (board.hasStack(side, sq))  ownT++;
        if (board.hasStack(enemy, sq)) oppT++;
    }
    score += 200 * (ownT - oppT);

    // 3) Zentrum (3×3-Mittelbereich)
    for (int r = 2; r <= 4; ++r) {
        for (int c = 2; c <= 4; ++c) {
            int sq = r*7 + c;
            if      (board.hasStack(side, sq) || board.hasGuard(side, sq))  score += 30;
            else if (board.hasStack(enemy, sq)|| board.hasGuard(enemy, sq)) score -= 30;
        }
    }

    // 4) Distanz Guard → Ziel
    score += (14 - dMy) * 50;

    // 5) Türme auf Guard-Linien
    int enemyG = -1;
    for (int sq = 0; sq < 49; ++sq) {
        if (board.hasGuard(enemy, sq)) enemyG = sq;
    }
    int ogr = enemyG/7, ogc = enemyG%7;
    for (int sq = 0; sq < 49; ++sq) {
        if (board.hasStack(side, sq)) {
            int r = sq/7, c = sq%7;
            if (r == ogr || c == ogc) score += 40;
        }
        if (board.hasStack(enemy, sq)) {
            int r = sq/7, c = sq%7;
            if (r == ogr || c == ogc) score -= 40;
        }
    }

    // 6) Mobilität
    std::vector<::Move> ownM;
    MoveGenerator::generate(board, ownM);
    score += static_cast<int>(ownM.size()) * 5;

    // 7) Drohungen gegen Guard
    for (auto& m : ownM) {
        if (board.hasGuard(enemy, m.to)) score += 2000;
    }
    std::vector<::Move> oppM;
    MoveGenerator::generate(board, oppM);
    for (auto& m : oppM) {
        if (board.hasGuard(side, m.to))  score -= 2000;
    }

    return score;
}

} // namespace ai
