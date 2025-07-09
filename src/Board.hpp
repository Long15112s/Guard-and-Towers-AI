#pragma once
#include <cstdint>
#include <array>
#include <string>
#include "FEN.hpp"

enum class Color { Red=0, Blue=1, None=2 };

struct Move {
    uint8_t from, to;
    uint8_t splitCount;  // 0=Guard, >0=Stack‑Move
};

class MoveGenerator;  // forward

class Board {
public:
    Board();
    bool loadFEN(const std::string& fen);
    std::string toFEN() const;
    void print() const;
    bool makeMove(const Move& mv);
    bool isGameOver() const;
    Color winner() const;
    Color sideToMove() const { return side; }

    // GUI‑Hilfen
    bool hasGuard(Color c,int sq) const { return guardBB[int(c)] & (1ULL<<sq); }
    bool hasStack(Color c,int sq) const { return stackBB[int(c)] & (1ULL<<sq); }
    int  heightAt(int sq) const { return height[sq]; }

private:
    bool parsePieceRow(const std::string& row,int rank);

    std::array<uint8_t,49> height;   // 0..7, 255=Guard
    uint64_t guardBB[2];             // Guards Red=0,Blue=1
    uint64_t stackBB[2];             // Stacks Red=0,Blue=1
    uint64_t occAll;                 // alle besetzten Felder
    Color   side;                    // wer ist am Zug

    friend class MoveGenerator;
};
