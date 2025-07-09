#pragma once

#include "AI.hpp"
#include "../Board.hpp"
#include "../MoveGenerator.hpp"
#include <unordered_map>
#include <vector>
#include <chrono>
#include <limits>
#include <random>
#include <cstdint>

namespace ai {

//
// --- Flag-Typen für Transposition Table (TT) ---
enum class TTFlag : uint8_t {
    EXACT,       // exakter Wert
    LOWERBOUND,  // Wert ist eine untere Schranke (Beta-Cutoff)
    UPPERBOUND   // Wert ist eine obere Schranke (Alpha-Cutoff)
};

//
// --- Struktur für einen Eintrag im Transposition Table ---
struct TTEntry {
    uint64_t key;       // Zobrist-Hash der Stellung
    int      depth;     // Resttiefe, für die der Eintrag gültig ist
    int      value;     // Bewertungswert (Score) für diese Stellung
    TTFlag   flag;      // EXACT / LOWERBOUND / UPPERBOUND
    ::Move   bestMove;  // Bester Zug in dieser Stellung (für Move-Ordering)
};

//
// --- TT_PVSAI: Kombiniert Transposition Table, PVS, Killer, MVV-LVA und History-Heuristik ---
//
class TT_PVSAI : public AI {
public:
    // maxDepth: maximale Suchtiefe, maxTimeMs: Timeout in Millisekunden
    explicit TT_PVSAI(int maxDepth = 8, int maxTimeMs = 120000);

    // Wählt den besten Zug in der gegebenen Stellung für die Seite 'side'
    ::Move chooseMove(const ::Board& board, ::Color side) override;

    // Ermöglicht Auslesen der Knotenzahl (zum Benchmark)
    uint64_t getNodeCount() const { return _nodeCount; }

private:
    //---------------------------------------------
    // 1) Root-Funktion: Iteratives Deepening + PVS
    //---------------------------------------------
    ::Move rootSearch(const ::Board& board, int depth, ::Color side);

    //---------------------------------------------
    // 2) PVS-Suche mit TT und Killer-Heuristik
    //---------------------------------------------
    int pvsWithTT(::Board& board,
                  int depth,
                  int alpha,
                  int beta,
                  ::Color side,
                  bool maximizing);

    //---------------------------------------------
    // 3) Zobrist-Hashing (TT-Key)
    //---------------------------------------------
    uint64_t computeHash(const ::Board& board, ::Color side) const;
    void initZobrist(); // Initialisiert die statischen Zufalls-Tabellen

    //---------------------------------------------
    // 4) Move-Ordering 
    //    (TT-bestMove → Killer → Captures (MVV-LVA) → History → Rest)
    //---------------------------------------------
    void orderMoves(const ::Board& board,
                    std::vector<::Move>& moves,
                    const TTEntry* ttHit,
                    int currentDepth,
                    ::Color side) const;

    //---------------------------------------------
    // 5) Evaluation (identisch zu AlphaBetaAI/PVSAI)
    //---------------------------------------------
    int eval(const ::Board& board, ::Color side) const;

    //---------------------------------------------
    // 6) Timeout-Check
    //---------------------------------------------
    bool isTimeout() const;

private:
    int      _maxDepth;
    int      _maxTimeMs;
    uint64_t _nodeCount;
    uint64_t _startTimeMs;

    // Transposition Table: Hash → Eintrag
    std::unordered_map<uint64_t, TTEntry> _tt;

    // Zobrist-Tabellen (49 Felder × 4 Piece-Typen) + Side-Hash
    static bool     _zobristInited;
    static uint64_t _zTable[49][4];
    static uint64_t _zSide;

    // Killer-Heuristik: maximal 2 Killer-Moves pro Tiefe
    static const int MAX_DEPTH_SUPPORTED   = 64;
    ::Move _killerMoves[MAX_DEPTH_SUPPORTED][2];

    // History-Tabelle: [From][To][Depth] → Heuristik-Score
    static const int MAX_MOVES            = 49 * 49 * MAX_DEPTH_SUPPORTED;
    mutable int _historyScore[MAX_MOVES];

    // Konstanten für Evaluation
    static constexpr int RED_GOAL_SQ       = 3;
    static constexpr int BLUE_GOAL_SQ      = 45;
};

} // namespace ai
