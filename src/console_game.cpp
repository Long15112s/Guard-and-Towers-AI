// src/console_game.cpp
#include <iostream>
#include <string>
#include <memory>
#include <limits>
#include <vector>
#include <algorithm>

#include "Board.hpp"
#include "MoveGenerator.hpp"
#include "AIRegistry.hpp"    // <–– statt einzelner Includes

static std::string readMove() {
    std::string mv;
    std::cout << "Dein Zug (z.B. A1-B1 oder q zum Aufgabe): ";
    std::getline(std::cin, mv);
    return mv;
}

int main() {
    // 1) Modus wählen
    std::cout << "Modus wählen:\n"
              << "  1) PvP\n"
              << "  2) PvE\n"
              << "  3) AIvAI\n"
              << "Eingabe: ";
    int mode;
    std::cin >> mode;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    bool humanRed = true;
    std::unique_ptr<ai::AI> aiRed, aiBlue;

    if (mode == 2) {
        // PvE: Modell und Tiefe wählen
        auto& reg = AIRegistry();

        std::cout << "KI auswählen:\n";
        for (size_t i = 0; i < reg.size(); ++i)
            std::cout << "  " << (i+1) << ") " << reg[i].name << "\n";

        int ki;
        std::cout << "Eingabe: ";
        std::cin >> ki;

        int depth;
        // Bei RandomAI Tiefe automatisch auf 0, sonst abfragen
        if (reg[ki-1].name == "ai::RandomAI") {
            depth = 0;
        } else {
            std::cout << "Tiefe für KI: ";
            std::cin >> depth;
        }

        std::cout << "Spieler spielt als (1=Rot, 2=Blau): ";
        int c;
        std::cin >> c;
        humanRed = (c == 1);

        // AI-Instanz erzeugen
        auto aiPtr = reg[ki-1].create(depth);
        if (humanRed)
            aiBlue = std::move(aiPtr);
        else
            aiRed  = std::move(aiPtr);
    }
    else if (mode == 3) {
        // AI-vs-AI: je KI Modell und Tiefe wählen
        auto& reg = AIRegistry();

        int kri, kdepth;
        // Rot
        std::cout << "Modell für Rot auswählen:\n";
        for (size_t i = 0; i < reg.size(); ++i)
            std::cout << "  " << (i+1) << ") " << reg[i].name << "\n";
        std::cout << "Eingabe: ";
        std::cin >> kri;
        if (reg[kri-1].name == "ai::RandomAI") {
            kdepth = 0;
        } else {
            std::cout << "Tiefe für Rot: ";
            std::cin >> kdepth;
        }
        aiRed = reg[kri-1].create(kdepth);

        // Blau
        std::cout << "Modell für Blau auswählen:\n";
        for (size_t i = 0; i < reg.size(); ++i)
            std::cout << "  " << (i+1) << ") " << reg[i].name << "\n";
        std::cout << "Eingabe: ";
        std::cin >> kri;
        if (reg[kri-1].name == "ai::RandomAI") {
            kdepth = 0;
        } else {
            std::cout << "Tiefe für Blau: ";
            std::cin >> kdepth;
        }
        aiBlue = reg[kri-1].create(kdepth);
    }

    // 2) Board initialisieren
    Board board;
    const std::string startFEN = "r1r11RG1r1r1/2r11r12/3r13/7/3b13/2b11b12/b1b11BG1b1b1 r";
    board.loadFEN(startFEN);

    // 3) Spielschleife
    while (!board.isGameOver()) {
        board.print();

        // Anzeige, wer am Zug ist
        auto side = board.sideToMove();
        std::cout << "Am Zug: " << (side == Color::Red ? "Rot\n" : "Blau\n");

        // Prüfen, ob menschlicher Zug
        bool humanTurn = (mode == 1)  // PvP
                      || (mode == 2 && ((side == Color::Red) == humanRed));  // PvE

        if (humanTurn) {
            // menschlicher Zug
            std::string input;
            Move mv;
            bool ok = false;
            do {
                input = readMove();
                if (input == "q") return 0;
                if (!MoveGenerator::parse(input, mv)) {
                    std::cout << "Ungültiges Format, bitte nochmal.\n";
                    continue;
                }
                std::vector<Move> legalMoves;
                MoveGenerator::generate(board, legalMoves);
                auto it = std::find_if(
                    legalMoves.begin(), legalMoves.end(),
                    [&](auto const& m) {
                        return m.from==mv.from
                            && m.to==mv.to
                            && m.splitCount==mv.splitCount;
                    }
                );
                if (it == legalMoves.end()) {
                    std::cout << "Illegaler Zug!\n";
                    continue;
                }
                ok = board.makeMove(mv);
                if (!ok)
                    std::cout << "MoveGenerator und Board disagreed - Zug fehlgeschlagen.\n";
            } while (!ok);
        } else {
            // AI-Zug
            Move aiMv;
            if (side == Color::Red)
                aiMv = aiRed  ? aiRed->chooseMove(board, side)  : Move{};
            else
                aiMv = aiBlue ? aiBlue->chooseMove(board, side) : Move{};

            if (aiMv.from >= 0) {
                std::cout << "AI (" << (side == Color::Red ? "Rot" : "Blau")
                          << ") spielt Zug "
                          << MoveGenerator::toString(aiMv) << "\n";
                board.makeMove(aiMv);
            } else {
                std::cout << "AI kann nicht ziehen.\n";
                break;
            }
        }
    }

    // Spielende
    board.print();
    auto win = board.winner();
    if (win == Color::None)
        std::cout << "Unentschieden.\n";
    else
        std::cout << (win == Color::Red ? "Rot" : "Blau") << " hat gewonnen!\n";

    return 0;
}
