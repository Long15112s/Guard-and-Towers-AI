// src/ai/MinimaxAI.hpp
#pragma once

#include "AI.hpp"
#include "../Board.hpp"
#include "../MoveGenerator.hpp"
#include <cstdint>
#include <vector>

namespace ai {

class MinimaxAI : public AI {
public:
    // maxDepth: maximale Suchtiefe, maxTimeMs: Timeout in ms
    explicit MinimaxAI(int maxDepth = 5, int maxTimeMs = 120000);

    ::Move chooseMove(const ::Board& board, ::Color side) override;

    int     eval(const ::Board& board, ::Color side) const;

    uint64_t getNodeCount() const { return _nodeCount; }

private:
    int      _maxDepth;
    int      _maxTimeMs;
    uint64_t _nodeCount;

    static constexpr int RED_GOAL_SQ  = 3;
    static constexpr int BLUE_GOAL_SQ = 45;

    ::Move minimaxRoot(const ::Board& board, int depth, ::Color side);
    int     minimax(::Board board, int depth, bool maximizing, ::Color side);
};

} // namespace ai
