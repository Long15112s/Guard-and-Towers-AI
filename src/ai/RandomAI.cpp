// src/ai/RandomAI.cpp
#include "RandomAI.hpp"
#include "../MoveGenerator.hpp"
#include "AIRegistry.hpp"

REGISTER_AI(ai::RandomAI)

namespace ai {

RandomAI::RandomAI()
  : _rng(std::random_device{}())
{}

::Move RandomAI::chooseMove(const ::Board& board, ::Color side) {
    std::vector<::Move> all;
    MoveGenerator::generate(board, all);

    std::vector<::Move> own;
    for (auto &m : all) {
        if (board.sideToMove() == side && m.from >= 0)
            own.push_back(m);
    }

    if (own.empty())
        return ::Move{};  // kein legaler Zug vorhanden

    std::uniform_int_distribution<int> dist(0, int(own.size()) - 1);
    return own[dist(_rng)];
}

} // namespace ai
