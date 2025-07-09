// bench/bench_PVS_TT.cpp

#include "Board.hpp"
#include "MoveGenerator.hpp"
#include "ai/MinimaxAI.hpp"           // <-- für reine eval-Messung
#include "ai/AlphaBetaAI.hpp"
#include "ai/TranspositionTableAI.hpp"
#include "ai/PVSAI.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;
using namespace ai;
using Clock = chrono::high_resolution_clock;

int main() {
    // ← Hier Deine Stellung einstellen (FEN-String):
    const string fen = "2r12r1r1/r16/4b32/7/3r13/2RG4/b1b11BG2b1 b";

    // 1) Board laden
    Board board;
    if (!board.loadFEN(fen)) {
        cerr << "Ungültige FEN: " << fen << "\n";
        return 1;
    }
    Color side = board.sideToMove();

    // 2) Bewertungsfunktion messen (10 000×) MIT MinimaxAI, weil dort eval() öffentlich ist
    MinimaxAI evAI(1, 0);  // Tiefe=1, Timeout=0 (Timeout wird hier nicht benötigt)
    auto tv0 = Clock::now();
    for (int i = 0; i < 10000; ++i) {
        evAI.eval(board, side);  // MinimaxAI::eval ist public :contentReference[oaicite:0]{index=0}
    }
    auto tv1 = Clock::now();
    double evalMs = chrono::duration_cast<chrono::duration<double, milli>>(tv1 - tv0).count();

    // 3) Datencontainer vorbereiten
    const int MAX_DEPTH = 6;
    vector<uint64_t> abNodes(MAX_DEPTH + 1), ttNodes(MAX_DEPTH + 1), pvsNodes(MAX_DEPTH + 1);
    vector<double>   abTime(MAX_DEPTH + 1),  ttTime(MAX_DEPTH + 1),  pvsTime(MAX_DEPTH + 1);
    vector<string>   abBest(MAX_DEPTH + 1),  ttBest(MAX_DEPTH + 1),  pvsBest(MAX_DEPTH + 1);

    // 4) Alpha-Beta 1…MAX_DEPTH
    for (int d = 1; d <= MAX_DEPTH; ++d) {
        AlphaBetaAI ab(d, 120000);
        auto t0 = Clock::now();
        auto bestAb = ab.chooseMove(board, side);
        auto t1 = Clock::now();
        abNodes[d] = ab.getNodeCount();
        abTime[d]  = chrono::duration_cast<chrono::duration<double, milli>>(t1 - t0).count();
        abBest[d]  = MoveGenerator::toString(bestAb);
    }

    // 5) Transposition Table 1…MAX_DEPTH
    for (int d = 1; d <= MAX_DEPTH; ++d) {
        TranspositionTableAI tt(d, 120000);
        auto t0 = Clock::now();
        auto bestTT = tt.chooseMove(board, side);
        auto t1 = Clock::now();
        ttNodes[d] = tt.getNodeCount();
        ttTime[d]  = chrono::duration_cast<chrono::duration<double, milli>>(t1 - t0).count();
        ttBest[d]  = MoveGenerator::toString(bestTT);
    }

    // 6) PVS 1…MAX_DEPTH
    for (int d = 1; d <= MAX_DEPTH; ++d) {
        PVSAI pvs(d, 120000);
        auto t0 = Clock::now();
        auto bestPVS = pvs.chooseMove(board, side);
        auto t1 = Clock::now();
        pvsNodes[d] = pvs.getNodeCount();
        pvsTime[d]  = chrono::duration_cast<chrono::duration<double, milli>>(t1 - t0).count();
        pvsBest[d]  = MoveGenerator::toString(bestPVS);
    }

    // 7) Ausgabe

    // a) reine Eval-Zeit
    cout << fixed << setprecision(2);
    cout << "Evaluationsfunktion (10k× eval mit MinimaxAI): " << evalMs << " ms\n\n";

    // b) Tabelle
    const int W = 14;
    cout << left
         << setw(W) << "Tiefe"
         << setw(W) << "AB-Nodes"
         << setw(W) << "AB-Time(ms)"
         << setw(W) << "AB-Best"
         << setw(W) << "TT-Nodes"
         << setw(W) << "TT-Time(ms)"
         << setw(W) << "TT-Best"
         << setw(W) << "PVS-Nodes"
         << setw(W) << "PVS-Time(ms)"
         << setw(W) << "PVS-Best"
         << "\n";
    cout << string(W * 10, '-') << "\n";

    for (int d = 1; d <= MAX_DEPTH; ++d) {
        cout << setw(W) << d
             << setw(W) << abNodes[d]
             << setw(W) << abTime[d]
             << setw(W) << abBest[d]
             << setw(W) << ttNodes[d]
             << setw(W) << ttTime[d]
             << setw(W) << ttBest[d]
             << setw(W) << pvsNodes[d]
             << setw(W) << pvsTime[d]
             << setw(W) << pvsBest[d]
             << "\n";
    }

    return 0;
}
