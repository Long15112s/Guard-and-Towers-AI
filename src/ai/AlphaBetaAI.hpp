#pragma once
#include "AI.hpp"
#include "../Board.hpp"
#include "../MoveGenerator.hpp"
#include <cstdint>
#include <vector>

namespace ai {

class AlphaBetaAI : public AI {
public:
    explicit AlphaBetaAI(int maxDepth = 5, int maxTimeMs = 120000);
    ::Move chooseMove(const ::Board& board, ::Color side) override;
    uint64_t getNodeCount() const { return _nodeCount; }

private:
    int      _maxDepth;
    int      _maxTimeMs;
    uint64_t _nodeCount;
    static constexpr int RED_GOAL_SQ  = 3;
    static constexpr int BLUE_GOAL_SQ = 45;

    ::Move alphabetaRoot(const ::Board& board, int depth, ::Color side);
    int     alphabeta(::Board board,
                      int depth,
                      int alpha,
                      int beta,
                      bool maximizing,
                      ::Color side);
    int     eval(const ::Board& board, ::Color side) const;
};

} // namespace ai
