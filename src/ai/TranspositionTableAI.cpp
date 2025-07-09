#include "TranspositionTableAI.hpp"
#include "AIRegistry.hpp"

#include <chrono>
#include <climits>
#include <iostream>
#include <algorithm>
#include <cmath>

// Registrierung im Factory-Pattern
REGISTER_AI(ai::TranspositionTableAI)

namespace ai {

// statische Member initialisieren
bool     TranspositionTableAI::_zobristInitialized = false;
uint64_t TranspositionTableAI::_zTable[49][4]     = {{0}};
uint64_t TranspositionTableAI::_zSide               = 0;

// Konstruktor: Zetze Parameter und initialisiere ggf. Zobrist-Tabellen
TranspositionTableAI::TranspositionTableAI(int maxDepth, int maxTimeMs)
  : _maxDepth(maxDepth)
  , _maxTimeMs(maxTimeMs)
  , _nodeCount(0)
  , _startTimeMs(0)
{
    if (!_zobristInitialized) {
        initZobrist();
        _zobristInitialized = true;
    }
}

// Hilfsfunktion: Initialisiere Zobrist-Tabellen mit zufälligen 64-Bit-Werten
void TranspositionTableAI::initZobrist() {
    std::mt19937_64 rng(0xC0FFEE); // Fixer Seed für Determinismus
    std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());

    // Für jedes der 49 Felder und jeden der 4 Piece-Typen zufälligen Hash generieren
    for (int sq = 0; sq < 49; ++sq) {
        for (int pt = 0; pt < 4; ++pt) {
            _zTable[sq][pt] = dist(rng);
        }
    }
    // Zufallswert für Side-to-move (hier z.B. für 'Blue' am Zug)
    _zSide = dist(rng);
}

// Berechne Zobrist-Hash für aktuelle Stellung
uint64_t TranspositionTableAI::computeHash(const ::Board& board, ::Color side) const {
    uint64_t h = 0ULL;

    // Jede Position von 0..48 prüfen:
    // - hat roter Guard?
    // - hat blauer Guard?
    // - hat roter Stack?
    // - hat blauer Stack?
    for (int sq = 0; sq < 49; ++sq) {
        if (board.hasGuard(::Color::Red, sq)) {
            h ^= _zTable[sq][0];
        } else if (board.hasGuard(::Color::Blue, sq)) {
            h ^= _zTable[sq][1];
        } else if (board.hasStack(::Color::Red, sq)) {
            h ^= _zTable[sq][2];
        } else if (board.hasStack(::Color::Blue, sq)) {
            h ^= _zTable[sq][3];
        }
    }
    // Side-to-move einbeziehen (XOR nur, wenn z.B. Blue am Zug ist)
    if (side == ::Color::Blue) {
        h ^= _zSide;
    }
    return h;
}

// Die Root-Funktion mit Iterativem Tiefensuche und TT
::Move TranspositionTableAI::chooseMove(const ::Board& board, ::Color side) {
    std::vector<::Move> moves;
    MoveGenerator::generate(board, moves);
    if (moves.empty()) return ::Move{};

    ::Move finalBest = moves[0];
    int lastDepth = 0;

    // Startzeit merken (in ms)
    auto tAllStart = std::chrono::high_resolution_clock::now();
    _startTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                      tAllStart.time_since_epoch()).count();

    // Für jede zunehmende Tiefe
    for (int depth = 1; depth <= _maxDepth; ++depth) {
        // Timeout prüfen
        auto now = std::chrono::high_resolution_clock::now();
        uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
                           - _startTimeMs;
        if (elapsed > static_cast<uint64_t>(_maxTimeMs)) {
            std::cout << "[TT-Timeout] Stoppe nach Tiefe " << (depth - 1) << "\n";
            break;
        }

        // TT leeren, Node-Count zurücksetzen
        _tt.clear();
        _nodeCount = 0;

        // Suche durchführen
        auto t0 = std::chrono::high_resolution_clock::now();
        ::Move bestAtDepth = ttRoot(board, depth, side);
        auto t1 = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        std::cout << "[TT-IterDeep] depth=" << depth
                  << "  best="  << MoveGenerator::toString(bestAtDepth)
                  << "  nodes=" << _nodeCount
                  << "  time="  << ms << "ms\n";

        finalBest = bestAtDepth;
        lastDepth = depth;
    }

    std::cout << "[TT-IterDeep] letzte fertige Tiefe=" << lastDepth << "\n";
    return finalBest;
}

// Root-Funktion: probiere alle Züge, nutze Alphabeta/TT auf Tiefe-1
::Move TranspositionTableAI::ttRoot(const ::Board& board, int depth, ::Color side) {
    std::vector<::Move> moves;
    MoveGenerator::generate(board, moves);
    if (moves.empty()) return ::Move{};

    ::Move bestMove = moves[0];
    int bestScore = std::numeric_limits<int>::min();

    int alpha = std::numeric_limits<int>::min();
    int beta  = std::numeric_limits<int>::max();

    for (auto &m : moves) {
        ::Board tmp = board;
        tmp.makeMove(m);

        int score = ttSearch(tmp, depth - 1, alpha, beta, side, false);
        if (score > bestScore) {
            bestScore = score;
            bestMove  = m;
        }
        alpha = std::max(alpha, bestScore);
    }
    return bestMove;
}

// Die eigentliche TT-gestützte Alphabeta-Suche
int TranspositionTableAI::ttSearch(::Board& board,
                                   int depth,
                                   int alpha,
                                   int beta,
                                   ::Color side,
                                   bool maximizing) {
    // Timeout überprüfen
    auto now = std::chrono::high_resolution_clock::now();
    uint64_t currentMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    if (currentMs - _startTimeMs > static_cast<uint64_t>(_maxTimeMs)) {
        // Wenn Timeout, Rückfall auf Evaluation direkt
        ++_nodeCount;
        return eval(board, side);
    }

    // Hash berechnen und in TT suchen
    uint64_t key = computeHash(board, side);
    auto it = _tt.find(key);
    if (it != _tt.end()) {
        TTEntry &entry = it->second;
        if (entry.depth >= depth) {
            // Gültiges TT-Eintrag gefunden
            if (entry.flag == TTFlag::EXACT) {
                return entry.value;
            } else if (entry.flag == TTFlag::LOWERBOUND && entry.value > alpha) {
                alpha = entry.value;
            } else if (entry.flag == TTFlag::UPPERBOUND && entry.value < beta) {
                beta = entry.value;
            }
            if (alpha >= beta) {
                return entry.value;
            }
        }
    }

    // Leaf-Node oder Spielende
    if (depth == 0 || board.isGameOver()) {
        ++_nodeCount;
        int val = eval(board, side);
        // Speichere Leaf-Ergebnis ins TT
        TTEntry leafEntry;
        leafEntry.key      = key;
        leafEntry.depth    = depth;
        leafEntry.value    = val;
        leafEntry.flag     = TTFlag::EXACT;
        leafEntry.bestMove = ::Move{};
        _tt[key] = leafEntry;
        return val;
    }

    std::vector<::Move> moves;
    MoveGenerator::generate(board, moves);
    if (moves.empty()) {
        ++_nodeCount;
        int val = eval(board, side);
        // Speichere ins TT
        TTEntry entry;
        entry.key      = key;
        entry.depth    = depth;
        entry.value    = val;
        entry.flag     = TTFlag::EXACT;
        entry.bestMove = ::Move{};
        _tt[key] = entry;
        return val;
    }

    int originalAlpha = alpha;
    ::Move bestLocalMove{};
    int value;

    if (maximizing) {
        value = std::numeric_limits<int>::min();
        for (auto &m : moves) {
            ::Board tmp = board;
            tmp.makeMove(m);

            int score = ttSearch(tmp, depth - 1, alpha, beta, side, false);
            if (score > value) {
                value = score;
                bestLocalMove = m;
            }
            alpha = std::max(alpha, value);
            if (alpha >= beta) {
                // Beta-Cutoff
                break;
            }
        }
    } else {
        value = std::numeric_limits<int>::max();
        // Gegner-Seite (invertiere „maximizing“-Flag)
        for (auto &m : moves) {
            ::Board tmp = board;
            tmp.makeMove(m);

            int score = ttSearch(tmp, depth - 1, alpha, beta, side, true);
            if (score < value) {
                value = score;
                bestLocalMove = m;
            }
            beta = std::min(beta, value);
            if (beta <= alpha) {
                // Alpha-Cutoff
                break;
            }
        }
    }

    // TT–Eintrag erstellen / aktualisieren
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

// Evaluation identisch zu AlphaBetaAI (kopiert, um Konsistenz zu wahren)
int TranspositionTableAI::eval(const ::Board& board, ::Color side) const {
    ::Color enemy = (side == ::Color::Red ? ::Color::Blue : ::Color::Red);

    // 0) Guaranteed-Win-Check
    int myG = -1;
    for (int sq = 0; sq < 49; ++sq) {
        if (board.hasGuard(side, sq)) myG = sq;
    }
    int goal = (side == ::Color::Red ? RED_GOAL_SQ : BLUE_GOAL_SQ);
    int dMy  = std::abs(myG/7 - goal/7) + std::abs(myG%7 - goal%7);

    int dOpp = std::numeric_limits<int>::max();
    for (int sq = 0; sq < 49; ++sq) {
        if (board.hasGuard(enemy, sq) || board.hasStack(enemy, sq))
            dOpp = std::min(dOpp, std::abs(sq/7 - goal/7) + std::abs(sq%7 - goal%7));
    }
    if (dMy < dOpp) {
        // freier Lauf ins Ziel
        return +10000000 - dMy;
    }

    // 1) Terminal
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

    // 3) Zentrum (3×3)
    for (int r = 2; r <= 4; ++r) {
        for (int c = 2; c <= 4; ++c) {
            int sq = r*7 + c;
            if      (board.hasStack(side, sq) || board.hasGuard(side, sq))  score += 30;
            else if (board.hasStack(enemy, sq)|| board.hasGuard(enemy, sq)) score -= 30;
        }
    }

    // 4) Distanz Guard→Ziel
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
    for (auto &m : ownM) {
        if (board.hasGuard(enemy, m.to)) score += 2000;
    }
    std::vector<::Move> oppM;
    MoveGenerator::generate(board, oppM);
    for (auto &m : oppM) {
        if (board.hasGuard(side, m.to))  score -= 2000;
    }

    return score;
}

} // namespace ai
