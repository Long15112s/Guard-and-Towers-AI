// bench/bench_MMAB.cpp

#include "Board.hpp"
#include "MoveGenerator.hpp"
#include "ai/MinimaxAI.hpp"
#include "ai/AlphaBetaAI.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;
using namespace ai;
using Clock = chrono::high_resolution_clock;

int main() {
    // ← Hier Deine Stellung einstellen:
    const string fen = "7/3r13/1RG1r13/3b1BG2/7/7/7 r";

    // 1) Board laden
    Board board;
    if (!board.loadFEN(fen)) {
        cerr << "Ungültige FEN: " << fen << "\n";
        return 1;
    }
    Color side = board.sideToMove();

    // 2) Bewertungsfunktion messen (10 000×)
    MinimaxAI evAI(1, 0);
    auto tv0 = Clock::now();
    for (int i = 0; i < 10000; ++i) {
        evAI.eval(board, side);  // public oder via evalPublic
    }
    auto tv1 = Clock::now();
    double evalMs = chrono::duration_cast<chrono::duration<double, milli>>(tv1 - tv0).count();

    // 3) Datencontainer
    vector<uint64_t> mNodes(7), aNodes(7);
    vector<double>   mTime(7),   aTime(7);
    vector<string>   aBest(7);

    // 4) Minimax 1…6
    for (int d = 1; d <= 6; ++d) {
        MinimaxAI mi(d, 120000);
        auto t0 = Clock::now();
        mi.chooseMove(board, side);
        auto t1 = Clock::now();
        mNodes[d] = mi.getNodeCount();
        mTime[d]  = chrono::duration_cast<chrono::duration<double, milli>>(t1 - t0).count();
    }

    // 5) Alpha-Beta 1…6
    for (int d = 1; d <= 6; ++d) {
        AlphaBetaAI ab(d, 120000);
        auto u0 = Clock::now();
        auto bestAb = ab.chooseMove(board, side);
        auto u1 = Clock::now();
        aNodes[d] = ab.getNodeCount();
        aTime[d]  = chrono::duration_cast<chrono::duration<double, milli>>(u1 - u0).count();
        aBest[d]  = MoveGenerator::toString(bestAb);
    }

    // 6) Ausgabe

    // a) reine Eval-Zeit
    cout << fixed << setprecision(2);
    cout << "Bewertungsfunktion (10k× eval): " << evalMs << " ms\n\n";

    // b) Tabelle
    const int W = 12;
    cout << left
         << setw(W) << "Tiefe"
         << setw(W) << "M-Nodes"
         << setw(W) << "M-Time(ms)"
         << setw(W) << "A-Nodes"
         << setw(W) << "A-Time(ms)"
         << setw(W) << "A-Best"
         << "\n";
    cout << string(W * 6, '-') << "\n";

    for (int d = 1; d <= 6; ++d) {
        cout << setw(W) << d
             << setw(W) << mNodes[d]
             << setw(W) << mTime[d]
             << setw(W) << aNodes[d]
             << setw(W) << aTime[d]
             << setw(W) << aBest[d]
             << "\n";
    }

    return 0;
}
