// src/ai/RandomAI.hpp
#pragma once

#include "AI.hpp"
#include "../MoveGenerator.hpp"
#include "../Board.hpp"
#include <random>

namespace ai {

class RandomAI : public AI {
public:
    RandomAI();
    ::Move chooseMove(const ::Board& board, ::Color side) override;
private:
    std::mt19937 _rng;
};

} // namespace ai
