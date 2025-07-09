#include "MonteCarloAI.hpp"
#include "AIRegistry.hpp"

#include <cmath>
#include <limits>
#include <climits>

REGISTER_AI(ai::MonteCarloAI)

namespace ai {

// Factory
MonteCarloAI::MonteCarloAI()
  : MonteCarloAI(1000, std::sqrt(2.0))
{}

// Manueller Konstruktor
MonteCarloAI::MonteCarloAI(int iterations, double c)
  : _iterations(iterations)
  , _c(c)
  , _nodeCount(0)
  , _rng(std::random_device{}())
  , _evalAI(6, 0)
{}

// Iterative MCTS
::Move MonteCarloAI::chooseMove(const ::Board& board, ::Color side) {
    Node root{board, side, ::Move{}, nullptr};
    ::Color rootSide = side;

    for (int i = 0; i < _iterations; ++i) {
        Node* node = select(&root);
        if (node->visits > 0 && !node->state.isGameOver()) {
            expand(node);
            node = node->children[_rng() % node->children.size()].get();
        }
        double result = simulate(node->state, rootSide);
        backpropagate(node, result, rootSide);
    }
    return bestChildMove(&root);
}

// UCT-Selection
MonteCarloAI::Node* MonteCarloAI::select(Node* root) {
    Node* node = root;
    while (!node->children.empty()) {
        double bestUCT = -1e9;
        Node* bestN = nullptr;
        for (auto& cp : node->children) {
            Node* c = cp.get();
            double wi = c->wins, ni = c->visits, N = node->visits;
            double uct = (ni > 0)
                ? (wi/ni + _c * std::sqrt(std::log(N)/ni))
                : std::numeric_limits<double>::infinity();
            if (uct > bestUCT) {
                bestUCT = uct;
                bestN   = c;
            }
        }
        node = bestN;
    }
    return node;
}

// Expansion
void MonteCarloAI::expand(Node* node) {
    std::vector<::Move> moves;
    MoveGenerator::generate(node->state, moves);
    for (auto& m : moves) {
        ::Board next = node->state;
        next.makeMove(m);
        auto child = std::make_unique<Node>();
        child->state          = next;
        child->toMove         = (node->toMove == ::Color::Red ? ::Color::Blue : ::Color::Red);
        child->moveFromParent = m;
        child->parent         = node;
        node->children.push_back(std::move(child));
    }
}

// Simulation mit Depth-Limit und Greedy-Fallback
double MonteCarloAI::simulate(::Board state, ::Color rootSide) {
    ::Color turn = state.sideToMove();      // wer gerade am Zug ist
    const int MAX_PLY = 100;
    int ply = 0;

    while (ply < MAX_PLY) {
        // 1) Ende überprüfen: Guard auf Ziel oder Guard geschlagen?
        if (state.isGameOver()) {
            break;
        }

        // 2) Alle Züge generieren
        std::vector<::Move> moves;
        MoveGenerator::generate(state, moves);
        if (moves.empty()) break;

        // 3) Greedy‐Rollout: wähle Zug mit höchster statischer Bewertung
        int bestVal = std::numeric_limits<int>::min();
        std::vector<::Move> bestMoves;
        for (auto& m : moves) {
            ::Board cand = state;
            cand.makeMove(m);
            // sobald Guard-Capture oder Ziel erreicht, cand.isGameOver()==true
            int v = _evalAI.eval(cand, turn);
            if (v > bestVal) {
                bestVal = v;
                bestMoves = {m};
            } else if (v == bestVal) {
                bestMoves.push_back(m);
            }
        }

        ::Move choice = bestMoves[_rng() % bestMoves.size()];
        state.makeMove(choice);

        // 4) Nach jedem Zug sofort auf Spielende prüfen
        if (state.isGameOver()) {
            break;
        }

        // 5) Seitenwechsel & Zähler
        turn = (turn == ::Color::Red ? ::Color::Blue : ::Color::Red);
        ++_nodeCount;
        ++ply;
    }

    // 6) Auswertung: wer hat gewonnen?
    ::Color winner = state.winner();
    return (winner == rootSide ? 1.0 : 0.0);
}

// Backpropagation
void MonteCarloAI::backpropagate(Node* node, double result, ::Color rootSide) {
    for (Node* cur = node; cur; cur = cur->parent) {
        cur->visits++;
        if (cur->toMove != rootSide)
            cur->wins += result;
        else
            cur->wins += (1.0 - result);
    }
}

// Wähle im Root-Knoten Kind mit den meisten Visits
::Move MonteCarloAI::bestChildMove(Node* root) {
    Node* best = nullptr;
    int maxV = -1;
    for (auto& cp : root->children) {
        if (cp->visits > maxV) {
            maxV = cp->visits;
            best = cp.get();
        }
    }
    return best ? best->moveFromParent : ::Move{};
}

} // namespace ai
