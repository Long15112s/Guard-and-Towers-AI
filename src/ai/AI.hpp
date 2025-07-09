// src/ai/AI.hpp
#pragma once

#include "../MoveGenerator.hpp"   // vollständige Definition von struct Move
#include "../Board.hpp"           // Definition von enum class Color

namespace ai {

class AI {
public:
    virtual ~AI() = default;
    // exakte Signatur: globaler ::Move und globaler ::Color
    virtual ::Move chooseMove(const ::Board& board, ::Color side) = 0;
};

} // namespace ai
