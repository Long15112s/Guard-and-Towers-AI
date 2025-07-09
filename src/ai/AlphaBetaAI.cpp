#include "AlphaBetaAI.hpp"
#include "AIRegistry.hpp"

#include <chrono>
#include <climits>
#include <iostream>
#include <algorithm>  // std::min, std::max
#include <cmath>      // std::abs

REGISTER_AI(ai::AlphaBetaAI)

namespace ai {

// Hilfsfunktion für Manhattan-Distanz
static inline int manhattan(int a, int b) {
    return std::abs(a/7 - b/7) + std::abs(a%7 - b%7);
}

AlphaBetaAI::AlphaBetaAI(int maxDepth, int maxTimeMs)
  : _maxDepth(maxDepth)
  , _maxTimeMs(maxTimeMs)
  , _nodeCount(0)
{}

::Move AlphaBetaAI::chooseMove(const ::Board& board, ::Color side) {
    std::vector<::Move> moves;
    MoveGenerator::generate(board, moves);
    if (moves.empty()) return ::Move{};

    ::Move finalBest = moves[0];
    int lastDepth = 0;

    auto tAllStart = std::chrono::high_resolution_clock::now();
    for (int depth = 1; depth <= _maxDepth; ++depth) {
        // Timeout prüfen
        auto now     = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - tAllStart).count();
        if (elapsed > _maxTimeMs) {
            std::cout << "[Timeout] Stoppe nach Tiefe " << (depth - 1) << "\n";
            break;
        }

        _nodeCount = 0;
        auto t0 = now;
        ::Move bestAtDepth = alphabetaRoot(board, depth, side);
        auto t1 = std::chrono::high_resolution_clock::now();

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        std::cout << "[IterDeep] depth=" << depth
                  << "  best="  << MoveGenerator::toString(bestAtDepth)
                  << "  nodes=" << _nodeCount
                  << "  time="  << ms << "ms\n";

        finalBest = bestAtDepth;
        lastDepth = depth;
    }

    std::cout << "[IterDeep] letzte fertige Tiefe=" << lastDepth << "\n";
    return finalBest;
}

::Move AlphaBetaAI::alphabetaRoot(const ::Board& board, int depth, ::Color side) {
    std::vector<::Move> moves;
    MoveGenerator::generate(board, moves);

    ::Move bestMove = moves.empty() ? ::Move{} : moves[0];
    int bestScore = INT_MIN;
    int alpha = INT_MIN, beta = INT_MAX;

    for (auto &m : moves) {
        ::Board tmp = board;
        tmp.makeMove(m);
        int sc = alphabeta(tmp, depth - 1, alpha, beta, false, side);
        if (sc > bestScore) {
            bestScore = sc;
            bestMove  = m;
        }
        alpha = std::max(alpha, bestScore);
    }
    return bestMove;
}

int AlphaBetaAI::alphabeta(::Board board,
                           int depth,
                           int alpha,
                           int beta,
                           bool maximizing,
                           ::Color side) {
    

    if (depth == 0 || board.isGameOver()) {
        ++_nodeCount;
        return eval(board, side);
    }

    std::vector<::Move> moves;
    MoveGenerator::generate(board, moves);

    if (maximizing) {
        int value = INT_MIN;
        for (auto &m : moves) {
            ::Board tmp = board; tmp.makeMove(m);
            value = std::max(value, alphabeta(tmp, depth - 1, alpha, beta, false, side));
            alpha = std::max(alpha, value);
            if (alpha >= beta) break;
        }
        return value;
    } else {
        int value = INT_MAX;
        for (auto &m : moves) {
            ::Board tmp = board; tmp.makeMove(m);
            value = std::min(value, alphabeta(tmp, depth - 1, alpha, beta, true, side));
            beta = std::min(beta, value);
            if (beta <= alpha) break;
        }
        return value;
    }
}

int AlphaBetaAI::eval(const ::Board& board, ::Color side) const {
    ::Color enemy = (side == ::Color::Red ? ::Color::Blue : ::Color::Red);

    // 0) Guaranteed-Win-Check
    int myG = -1;
    for (int sq = 0; sq < 49; ++sq)
        if (board.hasGuard(side, sq)) myG = sq;
    int goal = (side == ::Color::Red ? RED_GOAL_SQ : BLUE_GOAL_SQ);
    int dMy  = manhattan(myG, goal);

    int dOpp = INT_MAX;
    for (int sq = 0; sq < 49; ++sq) {
        if (board.hasGuard(enemy, sq) || board.hasStack(enemy, sq))
            dOpp = std::min(dOpp, manhattan(sq, goal));
    }
    if (dMy < dOpp) {
        // freier Lauf ins Ziel
        return +10000000 - dMy;
    }

    // 1) Terminal
    if (board.isGameOver())
        return (board.winner() == side ? +1000000 : -1000000);

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
    for (int r = 2; r <= 4; ++r)
    for (int c = 2; c <= 4; ++c) {
        int sq = r*7 + c;
        if      (board.hasStack(side, sq) || board.hasGuard(side, sq))  score += 30;
        else if (board.hasStack(enemy, sq)|| board.hasGuard(enemy, sq)) score -= 30;
    }

    // 4) Distanz Guard→Ziel
    int enemyG = -1;
    for (int sq = 0; sq < 49; ++sq) {
        if (board.hasGuard(enemy, sq)) enemyG = sq;
    }
    score += (14 - manhattan(myG, goal)) * 50;

    // 5) Türme auf Guard-Linien
    int ogr = enemyG/7, ogc = enemyG%7;
    for (int sq = 0; sq < 49; ++sq) {
        if (board.hasStack(side, sq)) {
            int r = sq/7, c = sq%7;
            if (r==ogr || c==ogc) score += 40;
        }
        if (board.hasStack(enemy, sq)) {
            int r = sq/7, c = sq%7;
            if (r==ogr || c==ogc) score -= 40;
        }
    }

    // 6) Mobilität
    std::vector<::Move> ownM;
    MoveGenerator::generate(board, ownM);
    score += static_cast<int>(ownM.size()) * 5;

    // 7) Drohungen gegen Guard
    for (auto &m : ownM)
        if (board.hasGuard(enemy, m.to)) score += 2000;
    std::vector<::Move> oppM;
    MoveGenerator::generate(board, oppM);
    for (auto &m : oppM)
        if (board.hasGuard(side, m.to))  score -= 2000;

    return score;
}

} // namespace ai
