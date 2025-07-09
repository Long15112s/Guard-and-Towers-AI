#pragma once

#include "Board.hpp"
#include <vector>
#include <string>

class MoveGenerator {
public:
    static void generate(const Board& board, std::vector<Move>& out);
    static bool parse(const std::string& s, Move& mv);
    static std::string toString(const Move& mv);
};
