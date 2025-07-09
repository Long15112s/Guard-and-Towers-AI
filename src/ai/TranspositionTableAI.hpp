#pragma once

#include "AI.hpp"
#include "../Board.hpp"
#include "../MoveGenerator.hpp"
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <random>
#include <limits>

namespace ai {

// Flag-Typ für TT–Einträge
enum class TTFlag : uint8_t {
    EXACT,       // exakter Wert
    LOWERBOUND,  // Wert ist eine untere Schranke (Beta-Pruning)
    UPPERBOUND   // Wert ist eine obere Schranke (Alpha-Pruning)
};

// Struktur für einen Eintrag im Transposition Table
struct TTEntry {
    uint64_t key;       // Zobrist-Hash
    int depth;          // Tiefe, für die der Eintrag gültig ist
    int value;          // Bewertungswert
    TTFlag flag;        // Pruning-Flag
    ::Move bestMove;    // bester Zug in dieser Stellung
};

class TranspositionTableAI : public AI {
public:
    // maxDepth: maximale Suchtiefe, maxTimeMs: Timeout in ms
    explicit TranspositionTableAI(int maxDepth = 5, int maxTimeMs = 120000);

    ::Move chooseMove(const ::Board& board, ::Color side) override;

    uint64_t getNodeCount() const { return _nodeCount; }

private:
    // Hauptfunktionen
    ::Move ttRoot(const ::Board& board, int depth, ::Color side);
    int ttSearch(::Board& board,
                 int depth,
                 int alpha,
                 int beta,
                 ::Color side,
                 bool maximizing);

    // Hash-Berechnung
    uint64_t computeHash(const ::Board& board, ::Color side) const;
    void initZobrist(); // Initialisierung der Zobrist-Tabellen bei erstem Aufruf

    // Hilfsfunktionen
    int eval(const ::Board& board, ::Color side) const;

private:
    int             _maxDepth;
    int             _maxTimeMs;
    uint64_t        _nodeCount;
    uint64_t        _startTimeMs;        // Startzeit in ms für Timeout-Kontrolle

    // Transposition Table: key → Eintrag
    std::unordered_map<uint64_t, TTEntry> _tt;

    // Zobrist-Tabellen:
    // Für jedes Quadrat (0..48) je 4 mögliche "Piece-Typen":
    //   0 = Red Guard, 1 = Blue Guard, 2 = Red Stack, 3 = Blue Stack
    // außerdem ein Zufallswert für „Side to move“.
    static bool     _zobristInitialized;
    static uint64_t _zTable[49][4];
    static uint64_t _zSide;  // XOR, wenn 'Blue' am Zug (beispielsweise)

    // Konstanten für Eval
    static constexpr int RED_GOAL_SQ  = 3;
    static constexpr int BLUE_GOAL_SQ = 45;
};

} // namespace ai
