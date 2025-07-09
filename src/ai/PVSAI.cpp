#include "PVSAI.hpp"
#include "AIRegistry.hpp"

#include <iostream>
#include <cmath>

REGISTER_AI(ai::PVSAI)

namespace ai {

// Hilfsfunktion: aktuelle Zeit in ms seit Epoch
static uint64_t nowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

// Konstruktor: Parameter setzen, Node‐Count initialisieren
PVSAI::PVSAI(int maxDepth, int maxTimeMs)
  : _maxDepth(maxDepth),
    _maxTimeMs(maxTimeMs),
    _nodeCount(0),
    _startTimeMs(0)
{}

// Timeout‐Abfrage
bool PVSAI::isTimeout() const {
    uint64_t current = nowMs();
    return (current - _startTimeMs) > static_cast<uint64_t>(_maxTimeMs);
}

// chooseMove: Iteratives Deepening mit PVS
::Move PVSAI::chooseMove(const ::Board& board, ::Color side) {
    std::vector<::Move> moves;
    MoveGenerator::generate(board, moves);
    if (moves.empty()) return ::Move{};

    ::Move finalBest = moves[0];
    int lastDepth = 0;

    // Startzeit merken
    _startTimeMs = nowMs();

    // Iterative Deepening von 1 bis _maxDepth
    for (int depth = 1; depth <= _maxDepth; ++depth) {
        if (isTimeout()) {
            std::cout << "[PVS-Timeout] Stoppe nach Tiefe " << (depth - 1) << "\n";
            break;
        }

        _nodeCount = 0;
        auto t0 = std::chrono::high_resolution_clock::now();

        ::Move bestAtDepth = pvsRoot(board, depth, side);

        auto t1 = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

        std::cout << "[PVS-IterDeep] depth=" << depth
                  << "  best="  << MoveGenerator::toString(bestAtDepth)
                  << "  nodes=" << _nodeCount
                  << "  time="  << ms << "ms\n";

        finalBest = bestAtDepth;
        lastDepth = depth;
    }

    std::cout << "[PVS-IterDeep] letzte fertige Tiefe=" << lastDepth << "\n";
    return finalBest;
}

// Root‐Funktion: probiere alle Züge mit PVS auf Tiefe-1
::Move PVSAI::pvsRoot(const ::Board& board, int depth, ::Color side) {
    std::vector<::Move> moves;
    MoveGenerator::generate(board, moves);
    if (moves.empty()) return ::Move{};

    ::Move bestMove = moves[0];
    int bestScore = std::numeric_limits<int>::min();
    int alpha = std::numeric_limits<int>::min();
    int beta  = std::numeric_limits<int>::max();

    bool firstChild = true;
    for (auto& m : moves) {
        if (isTimeout()) break;

        ::Board tmp = board;
        tmp.makeMove(m);

        int score;
        if (firstChild) {
            // Erstes Kind: volles Fenster
            score = pvsSearch(tmp, depth - 1, alpha, beta, side, false);
            firstChild = false;
        } else {
            // Alle folgenden Kinder: Narrow‐Band PVS
            score = pvsSearch(tmp, depth - 1, alpha, alpha + 1, side, false);
            // Wenn Ergebnis tatsächlich zwischen alpha und beta liegt, erneute Suche mit vollem Fenster
            if (score > alpha && score < beta) {
                score = pvsSearch(tmp, depth - 1, alpha, beta, side, false);
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

// PVS‐Suche
int PVSAI::pvsSearch(::Board& board,
                     int depth,
                     int alpha,
                     int beta,
                     ::Color side,
                     bool maximizing)
{
    // Timeout‐Check
    if (isTimeout()) {
        ++_nodeCount;
        return eval(board, side);
    }

    // Terminal‐Bedingungen
    if (depth == 0 || board.isGameOver()) {
        ++_nodeCount;
        int val = eval(board, side);
        return val;
    }

    std::vector<::Move> moves;
    MoveGenerator::generate(board, moves);
    if (moves.empty()) {
        ++_nodeCount;
        int val = eval(board, side);
        return val;
    }

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
                score = pvsSearch(tmp, depth - 1, alpha, beta, side, false);
                firstChild = false;
            } else {
                // Narrow‐Band (Null‐Fenster)
                score = pvsSearch(tmp, depth - 1, alpha, alpha + 1, side, false);
                // Re-Search, falls nötig
                if (score > alpha && score < beta) {
                    score = pvsSearch(tmp, depth - 1, alpha, beta, side, false);
                }
            }

            if (score > value) {
                value = score;
                bestLocalMove = m;
            }
            alpha = std::max(alpha, value);
            if (alpha >= beta) {
                // Beta‐Cutoff
                break;
            }
        }
    } else {
        // Minimizing‐Seite
        value = std::numeric_limits<int>::max();
        bool firstChild = true;
        for (auto& m : moves) {
            if (isTimeout()) break;

            ::Board tmp = board;
            tmp.makeMove(m);

            int score;
            if (firstChild) {
                score = pvsSearch(tmp, depth - 1, alpha, beta, side, true);
                firstChild = false;
            } else {
                score = pvsSearch(tmp, depth - 1, beta - 1, beta, side, true);
                if (score > alpha && score < beta) {
                    score = pvsSearch(tmp, depth - 1, alpha, beta, side, true);
                }
            }

            if (score < value) {
                value = score;
                bestLocalMove = m;
            }
            beta = std::min(beta, value);
            if (beta <= alpha) {
                // Alpha‐Cutoff
                break;
            }
        }
    }

    ++_nodeCount;
    return value;
}

// Evaluation (kopiert von TranspositionTableAI / AlphaBetaAI)
int PVSAI::eval(const ::Board& board, ::Color side) const {
    ::Color enemy = (side == ::Color::Red ? ::Color::Blue : ::Color::Red);

    // 0) Guaranteed-Win-Check
    int myG = -1;
    for (int sq = 0; sq < 49; ++sq) {
        if (board.hasGuard(side, sq)) myG = sq;
    }
    int goal = (side == ::Color::Red ? 3 : 45);
    int dMy  = std::abs(myG/7 - goal/7) + std::abs(myG%7 - goal%7);

    int dOpp = std::numeric_limits<int>::max();
    for (int sq = 0; sq < 49; ++sq) {
        if (board.hasGuard(enemy, sq) || board.hasStack(enemy, sq))
            dOpp = std::min(dOpp, std::abs(sq/7 - goal/7) + std::abs(sq%7 - goal%7));
    }
    if (dMy < dOpp) {
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
