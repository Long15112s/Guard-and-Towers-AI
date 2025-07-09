#pragma once
#include "AI.hpp"
#include "../Board.hpp"
#include "../MoveGenerator.hpp"
#include "MinimaxAI.hpp"

#include <memory>
#include <random>
#include <vector>

namespace ai {

class MonteCarloAI : public AI {
public:
    MonteCarloAI();                             // Factory-Wrapper
    MonteCarloAI(int iterations, double c);     // manuell

    ::Move chooseMove(const ::Board& board, ::Color side) override;
    uint64_t getNodeCount() const { return _nodeCount; }

private:
    struct Node {
        ::Board state;
        ::Color toMove;
        ::Move  moveFromParent{};
        Node* parent = nullptr;
        std::vector<std::unique_ptr<Node>> children;
        int visits = 0;
        double wins = 0.0;
    };

    Node* select(Node* root);
    void  expand(Node* node);
    double simulate(::Board state, ::Color toMove);
    void  backpropagate(Node* node, double result, ::Color rootSide);
    ::Move bestChildMove(Node* root);

    int              _iterations;
    double           _c;
    uint64_t         _nodeCount;
    std::mt19937_64  _rng;
    MinimaxAI        _evalAI;  // depth=1, timeout=0 → eval() öffentlich
};

} // namespace ai
