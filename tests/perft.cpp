#include "Board.hpp"
#include "MoveGenerator.hpp"
#include <cstdint>
#include <iostream>
// START: "r1r11RG1r1r1/2r11r12/3r13/7/3b13/2b11b12/b1b11BG1b1b1 r"
// Perft‑Zähler
uint64_t perft(const Board& board, int depth) {
    // Wenn Spiel zu Ende ist, zähle diese Stellung als ein Leaf
    if (board.isGameOver())  
        return 1;

    if (depth == 0) 
        return 1;

    std::vector<Move> moves;
    MoveGenerator::generate(board, moves);
    uint64_t nodes = 0;
    for (auto& mv : moves) {
        Board tmp = board;
        tmp.makeMove(mv);
        nodes += perft(tmp, depth - 1);
    }
    return nodes;
}

// Root‑Breakdown („Divide“) für eine feste Tiefe
void perftDivide(const Board& board, int depth) {
    std::vector<Move> moves;
    MoveGenerator::generate(board, moves);
    uint64_t sum = 0;
    std::cout << "Root‑Divide depth " << depth << ":\n";
    std::cout << "Move\tNodes\n";
    for (auto& mv : moves) {
        Board tmp = board;
        tmp.makeMove(mv);
        uint64_t cnt = perft(tmp, depth - 1);
        sum += cnt;
        std::cout
            << MoveGenerator::toString(mv)
            << "\t" << cnt << "\n";
    }
    std::cout << "Sum\t" << sum << "\n";
}

int main(int argc, char** argv) {
    // FEN aus Argument oder Standard‑Start
    const std::string startFEN = "r1r11RG1r1r1/2r11r12/7/3r13/3b13/2b11b12/b1b11BG1b1b1 b";
    std::string fen = (argc >= 2 ? argv[1] : startFEN);
    int maxDepth    = 6;
    if (argc >= 3) maxDepth = std::stoi(argv[2]);

    Board board;
    if (!board.loadFEN(fen)) {
        std::cerr << "Invalid FEN\n";
        return 1;
    }

    // Gesamt‑Perft für Tiefen 1…maxDepth
    for (int d = 1; d <= maxDepth; ++d) {
        uint64_t nodes = perft(board, d);
        std::cout << "Perft " << d << "\t" << nodes << "\n";
    }

    // Pro‑Move‑Breakdown nur für Tiefe = maxDepth
    std::cout << "\n";
    perftDivide(board, maxDepth);

    return 0;
}
