// src/console_main.cpp
#include <iostream>
#include <string>
#include <vector>
#include "Board.hpp"
#include "MoveGenerator.hpp"

int main(){
    // Beispiel‐FEN, du kannst hier auch per std::getline einlesen
    std::string fen = "r1r11RG1r1r1/2r11r12/3r13/7/3b13/2b11b12/b1b11BG1b1b1 r";

    // Board initialisieren und FEN laden
    Board b;
    if(!b.loadFEN(fen)){
        std::cerr << "Ungültige FEN\n";
        return 1;
    }

    // Brett ausgeben
    b.print();

    // Alle legalen Züge generieren
    std::vector<Move> mv;
    MoveGenerator::generate(b, mv);
    std::cout << "Legal moves (" << mv.size() << "):\n";
    for(auto &m : mv)
        std::cout << "  " << MoveGenerator::toString(m) << "\n";

    return 0;
}
