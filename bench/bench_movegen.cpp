#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include "Board.hpp"
#include "MoveGenerator.hpp"

// FEN‑Stellungen
static const std::vector<std::pair<std::string,std::string>> positions = {
    { "Startstellung",
      "r1r11RG1r1r1/2r11r12/3r13/7/3b13/2b11b12/b1b11BG1b1b1 r" },
    { "Mittelspiel",
      "7/2RG2r21/7/r11b21b12/1r25/3BG1b21/7 b" },
    { "Endspiel",
      "7/1b25/3BG3/1r22b12/7/2RG4/7 r" }
};

int main(){
    using Clock = std::chrono::high_resolution_clock;
    const int REPEATS = 10000;
    
    std::cout << "Benchmark MoveGenerator ("
              << REPEATS << "×)\n\n";

    for(auto &p : positions){
        const auto &name = p.first;
        const auto &fen  = p.second;

        // einmal laden prüfen
        Board board;
        if(!board.loadFEN(fen)){
            std::cerr<<"Ungültige FEN: "<<fen<<"\n";
            return 1;
        }

        // Warm‑up
        std::vector<Move> moves;
        MoveGenerator::generate(board, moves);

        // messen
        auto t0 = Clock::now();
        for(int i=0;i<REPEATS;++i){
            MoveGenerator::generate(board, moves);
        }
        auto t1 = Clock::now();
        double ms = std::chrono::duration<double,std::milli>(t1-t0).count();

        std::cout
          << name << ": "
          << ms << " ms total, "
          << (ms/REPEATS) << " ms per call\n";
    }
    return 0;
}
