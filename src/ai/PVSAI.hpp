#pragma once

#include "AI.hpp"
#include "../Board.hpp"
#include "../MoveGenerator.hpp"
#include <vector>
#include <chrono>
#include <limits>

namespace ai {

class PVSAI : public AI {
public:
    // maxDepth: maximale Suchtiefe, maxTimeMs: Timeout in ms
    explicit PVSAI(int maxDepth = 5, int maxTimeMs = 120000);

    ::Move chooseMove(const ::Board& board, ::Color side) override;

    uint64_t getNodeCount() const { return _nodeCount; }

private:
    // Root-Funktion mit Iterativem Deepening und PVS
    ::Move pvsRoot(const ::Board& board, int depth, ::Color side);

    // PVS-Suche: depth = verbleibende Resttiefe, alpha/beta Fenster, maximizing‐Flag
    int pvsSearch(::Board& board,
                  int depth,
                  int alpha,
                  int beta,
                  ::Color side,
                  bool maximizing);

    // Evaluation (identisch zu TranspositionTableAI / AlphaBetaAI)
    int eval(const ::Board& board, ::Color side) const;

    // Timeout‐Hilfe
    bool isTimeout() const;

private:
    int      _maxDepth;
    int      _maxTimeMs;
    uint64_t _nodeCount;
    uint64_t _startTimeMs;
};

} // namespace ai
