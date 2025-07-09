// gui/pages/GamePage_events.cpp
#include "GamePage.hpp"
#include "../../src/FEN.hpp"

#include "../../src/Board.hpp"
#include "../../src/MoveGenerator.hpp"  // for MoveGenerator::legalMoves()
#include "NetworkConnector.hpp"
#include "ai/MinimaxAI.hpp"

#include <iostream>
#include <cstring>
#include <algorithm>
#include <cmath>

static constexpr float ENTRY_H = /* two lines per entry */ 20.f * 2; 


using namespace sf;
using namespace std;

namespace {
    // this must match the offset you use in render.cpp
    static constexpr float PANEL_OFFSET_X = 50.f;

    // helper: from oldB and newB, find which single move by ‘mover’ transforms oldB→newB
    static ::Move detectOpponentMove(const ::Board& oldB, const ::Board& newB, ::Color mover) {
        std::vector<Move> all;
        MoveGenerator::generate(oldB, all);
        for (auto &m : all) {
            Board tmp = oldB;                // copy old position
            if (tmp.makeMove(m)               // apply move
            && tmp.toFEN() == newB.toFEN())  // exactly matches new position
            {
                return m;
            }
        }
        // fallback: return a “null” move (stack-move from==to) if nothing found
        return Move{0,0,0};
    }
}

namespace pages {


void GamePage::setNetworkConfig(const std::string& host, unsigned port, int depth) {
    _srvHost = host;
    _srvPort = port;
    _networkAIDepth = depth;
    // falls schon ein Connector da ist, zurücksetzen:
    resetNetwork();
}

void GamePage::setNetworkAI(std::unique_ptr<ai::AI> ai) {
    _networkAI = std::move(ai);
}

void GamePage::resetNetwork() {
  _net.reset();
  _playerId = -1;
  _board = Board();
}


void GamePage::applyMoveLocally(const Move& mv, ::Color mover) {

    // apply to our board
    _board.makeMove(mv);

    // push new FEN into the text‐field
    std::string newFen = _board.toFEN();
    _fenField.setString(newFen);

    // trim any “future” redo‐history
    if (_selectedHistoryIndex < int(_history.size()) - 1)
        _history.resize(_selectedHistoryIndex + 1);

    // record this move
    _history.emplace_back(mv, newFen, mover);
    _selectedHistoryIndex = int(_history.size()) - 1;

    // flip side
    _side = _board.sideToMove();

    // check for guard‐capture or goal
    bool guardCaptured = _board.hasGuard(static_cast<::Color>(1 - int(mover)), mv.to);
    if (guardCaptured) {
        _gameOver = true;
        _winMessage = (mover == ::Color::Red
            ? "Rot hat den Wächter gefangen."
            : "Blau hat den Wächter gefangen.");
    } else if ((mover == ::Color::Red && mv.to == RED_GOAL_SQ) ||
               (mover == ::Color::Blue && mv.to == BLUE_GOAL_SQ)) {
        _gameOver = true;
        _winMessage = (mover == ::Color::Red
            ? "Rot hat das Ziel erreicht."
            : "Blau hat das Ziel erreicht.");
    }
}





void GamePage::runAIMove() {
    if (_mode != Mode::HumanVsAI || _gameOver)
        return;

    ai::AI* currentAI =
        (_side == ::Color::Red ? _redAI.get() : _blueAI.get());
    if (!currentAI)
        return;

    Move aiMove = currentAI->chooseMove(_board, _side);
    if (aiMove.from < 0) return;

    ::Color mover = _side;

    bool willCaptureGuard = _board.hasGuard(
        static_cast<::Color>(1 - int(_side)),
        aiMove.to
    );

    _board.makeMove(aiMove);
    _moveSound.play();
    std::string newFen = _board.toFEN();
    _fenField.setString(newFen);

    _history.emplace_back(aiMove, newFen, mover);



    if (_selectedHistoryIndex < int(_history.size()) - 1) {
        _history.resize(_selectedHistoryIndex + 1);
    }
    _history.emplace_back(aiMove, newFen, mover);
    _selectedHistoryIndex = int(_history.size()) - 1;

    _side = _board.sideToMove();

    if (willCaptureGuard) {
        _gameOver = true;
        _winMessage = (mover == ::Color::Red
            ? "Rot hat den Wächter gefangen."
            : "Blau hat den Wächter gefangen.");
    }
    // neu: Wächter-Ziel erreichen
    else if (mover == ::Color::Red && aiMove.to == RED_GOAL_SQ) {
        _gameOver   = true;
        _winMessage = "Rot hat das Ziel erreicht.";
    }
    else if (mover == ::Color::Blue && aiMove.to == BLUE_GOAL_SQ) {
        _gameOver   = true;
        _winMessage = "Blau hat das Ziel erreicht.";
    }
}

void GamePage::handleEvent(const sf::Event& ev) {
    _fenField.handleEvent(ev, _win);

    float panelX = _M_L + 7*_TILE + GuiStyle::PANEL_PADDING + PANEL_OFFSET_X + 50.f;
    float panelY = _M_T + 70.f; 
    float panelW = _win.getSize().x - panelX - GuiStyle::PANEL_PADDING;
    float panelH = 6*_TILE; 


    if (ev.type == sf::Event::MouseButtonPressed) {
        Vector2f p = _win.mapPixelToCoords({ev.mouseButton.x, ev.mouseButton.y});
        // Load
        if (_loadBtn.getGlobalBounds().contains(p)) {
            _gameOver = false;
            _winMessage.clear();
            string fen = _fenField.getString();
            fen.erase(remove(fen.begin(), fen.end(), '\r'), fen.end());
            auto l = fen.find_first_not_of(" \t\n");
            auto r = fen.find_last_not_of(" \t\n");
            fen = (l==string::npos? "" : fen.substr(l, r-l+1));
            auto sp = fen.find(' ');
            string placement = fen.substr(0, sp);
            string side = (sp==string::npos? "" : fen.substr(sp+1));

            if (hasSoldierWithoutHeight(placement)) {
                cerr << "Ungültiges FEN: Soldat ohne Höhe\n";
                return;
            }
            string fixedPlacement = padPlacement(placement);
            if (!validateFENPlacement(fixedPlacement)) {
                cerr << "Ungültiges FEN-Format (Placement)\n";
                return;
            }
            string toLoad = fixedPlacement + (side.empty() ? "" : " "+side);
            cerr << "[Load] versuche temporär zu laden: [" << toLoad << "]\n";

            Board tmpBoard;
            if (!tmpBoard.loadFEN(toLoad)) {
                cerr << "Parser-Fehler bei loadFEN, Spiel bleibt unverändert\n";
                return;
            }
            _board = move(tmpBoard);
            string confirmed = _board.toFEN();
            _fenField.setString(confirmed);
            _side = _board.sideToMove();
            
            cout << "Erfolgreich geladen: [" << confirmed << "]\n";
            // History zurücksetzen
            _history.clear();
            _history.emplace_back(Move{}, confirmed, _side);
            _selectedHistoryIndex = 0;
            _historyScrollOffset = 0.f;
            if (_mode == Mode::HumanVsAI) {
                // _humanIsRed == true  → Mensch spielt Rot
                // _humanIsRed == false → Mensch spielt Blau
                ::Color humanSide = _humanIsRed ? ::Color::Red : ::Color::Blue;
                // Falls aktuelle Seite (_side) nicht der Mensch ist, ist es die KI
                if (_side != humanSide) {
                    runAIMove();
                }
            }
            return;
        }

        // New Game
        if (_newGameBtn.getGlobalBounds().contains(p)) {
            // Basis‐FEN
            std::string fen = _startFen;
            // Nur im Human-vs-AI, wenn Mensch Blau spielt:
            if (_mode == Mode::HumanVsAI && !_humanIsRed) {
                // letztes Zeichen tauschen: 'r' <-> 'b'
                if (!fen.empty()) {
                    char &c = fen.back();
                    c = (c == 'r' ? 'b' : 'r');
                }
            }
            // Lade das (ggf. angepasste) FEN
            _board.loadFEN(fen);
            _side = _board.sideToMove();
            _fenField.setString(fen);
            _gameOver = false;
            _winMessage.clear();
            // History zurücksetzen
            _history.clear();
            _history.emplace_back(Move{}, fen, _side);
            _selectedHistoryIndex = 0;
            _historyScrollOffset = 0.f;
            return;
        }
    }
    // 0) Mausrad scrollt nur, wenn der Cursor über der History-Panel ist
    if (ev.type == sf::Event::MouseWheelScrolled) {
        sf::Vector2f mp = _win.mapPixelToCoords({ev.mouseWheelScroll.x, ev.mouseWheelScroll.y});
        if (mp.x >= panelX && mp.x <= panelX + panelW
        && mp.y >= panelY && mp.y <= panelY + panelH) {
            // 1) Offset um eine Zeile verschieben
            _historyScrollOffset -= ev.mouseWheelScroll.delta * GuiStyle::HISTORY_LINE_H;

            // 2) Grenzen berechnen und clampen
            float contentH = _history.size()*ENTRY_H + 2*GuiStyle::HISTORY_PADDING;
            float maxOff   = std::max(0.f, contentH - panelH);
            _historyScrollOffset = std::clamp(_historyScrollOffset, 0.f, maxOff);

            // 3) Auf Vielfaches der Zeilenhöhe runden, um Springen zu vermeiden
            _historyScrollOffset =
                std::round(_historyScrollOffset / GuiStyle::HISTORY_LINE_H) * GuiStyle::HISTORY_LINE_H;
            return;
        }
    }
    else if (ev.type == sf::Event::MouseButtonReleased) {
        sf::Vector2f mp = _win.mapPixelToCoords({ev.mouseButton.x, ev.mouseButton.y});

        // history‐panel?
        if (mp.x >= panelX && mp.x <= panelX + panelW &&
            mp.y >= panelY && mp.y <= panelY + panelH)
        {
            float relY = (mp.y - panelY - float(GuiStyle::HISTORY_PADDING)) + _historyScrollOffset;
            int idx = int(relY / ENTRY_H);          // now using ENTRY_H
            if (idx >= 0 && idx < (int)_history.size()) {
                // store old selection to require double-click for AI
                int oldIdx = _selectedHistoryIndex;
                
                // update to chosen history entry
                _selectedHistoryIndex = idx;
                const auto& e = _history[idx];
                _board.loadFEN(e.fen);
                _fenField.setString(e.fen);
                _side = (e.mover == ::Color::Red ? ::Color::Blue : ::Color::Red);
                _gameOver = false;
                if (_mode == Mode::HumanVsAI
                 && _side != (_humanIsRed ? ::Color::Red : ::Color::Blue)
                 && idx == oldIdx)
                {
                    runAIMove();
                }
            }
            return;
        }

        // Back button
        if (_backBtn.getGlobalBounds().contains(mp)) {
            _active = false;
            return;
        }
    }

    // wenn KI-vs-KI: nur UI, dann keine Drag-Drop-Events
    if (_mode == Mode::AIvsAI)
        return;

    if (_mode == Mode::Network && !_networkManual)
        return;

    // 2) Keine Züge mehr, wenn Spiel vorbei
    if (_gameOver)
        return;

    
    // 3) Drag & Drop
    if (ev.type == sf::Event::MouseButtonPressed && !_dragging) {
        int sq = pixelToSq({ev.mouseButton.x, ev.mouseButton.y});
        _dragMovedCount = 0;
        _dragHoverTo    = -1;
        if (sq >= 0 && (_board.hasGuard(_side,sq) || _board.hasStack(_side,sq))) {
            _dragging    = true;
            _fromSq      = sq;
            computeLegal(sq);
            _dragIsGuard = _board.hasGuard(_side, sq);
            sf::Color dragCol = (_side==::Color::Red ? sf::Color::Red : sf::Color::Blue);

            if (_dragIsGuard) {
                _dragRect.setSize({_TILE-10.f, _TILE-10.f});
                _dragRect.setOrigin(_dragRect.getSize().x/2.f, _dragRect.getSize().y/2.f);
                _dragRect.setFillColor(dragCol);
                _dragRect.setOutlineColor(sf::Color::Black);
                _dragRect.setOutlineThickness(2.f);
                _dragRect.setPosition(sqToPos(sq));
            } else {
                int srcH = _board.heightAt(sq);
                _dragText.setString(to_string(srcH));
                _dragText.setOrigin(_dragText.getLocalBounds().width/2,
                                    _dragText.getLocalBounds().height/2 + _dragText.getLocalBounds().top);
                _dragShape.setRadius(_TILE/2.f - 4.f);
                _dragShape.setOrigin(_dragShape.getRadius(), _dragShape.getRadius());
                _dragShape.setOutlineColor(sf::Color::Black);
                _dragShape.setOutlineThickness(2.f);
                _dragShape.setPosition(sqToPos(sq));
                _dragText.setString("");
            }
        }
    }
    else if (ev.type == sf::Event::MouseMoved && _dragging) {
        sf::Vector2f mpos(float(ev.mouseMove.x), float(ev.mouseMove.y));
        if (_dragIsGuard) {
            _dragRect.setPosition(mpos);
        } else {
            _dragShape.setPosition(mpos);
            int toSq = pixelToSq({ev.mouseMove.x, ev.mouseMove.y});
            bool overLegal = false;
            for (auto &m : _legalMoves) {
                if (m.to == toSq) {
                    overLegal = true;
                    _dragHoverTo    = toSq;
                    _dragMovedCount = m.splitCount;
                    int destH = _board.heightAt(toSq);
                    bool enemyStack = _board.hasStack(_side == ::Color::Red   ? ::Color::Blue : ::Color::Red, toSq);                    
                    bool ownStack   = _board.hasStack(_side, toSq);
                    int moved = m.splitCount;
                    int finalH = enemyStack ? moved : ownStack ? destH+moved : moved;
                    _dragText.setString(to_string(finalH));
                    auto lb = _dragText.getLocalBounds();
                    _dragText.setOrigin(lb.width/2, lb.height/2 + lb.top);
                    _dragText.setPosition(mpos);
                    break;
                }
            }
            if (!overLegal) {
                _dragText.setString("");
                _dragHoverTo    = -1;
                _dragMovedCount = 0;
            }
        }
    }
    else if (ev.type == sf::Event::MouseButtonReleased && _dragging) {
        int toSq = pixelToSq({ev.mouseButton.x, ev.mouseButton.y});
        bool applied = false;
        Move appliedMove{}; 
        // … innerhalb deines MouseButtonReleased-Handlers …
        for (auto &m : _legalMoves) {
            if (m.to != toSq) continue;

            // 1) Wer zieht gerade?
            ::Color mover = _side;

            // 2) Wird ein Wächter geschlagen?
            bool willCaptureGuard = _board.hasGuard(
                static_cast<::Color>(1 - int(_side)), m.to
            );

            // 3) Führe den Zug aus
            if (_board.makeMove(m)) {
                _moveSound.play();
                applied = true;
                appliedMove = m;
                _fenField.setString(_board.toFEN());
                std::string newFen = _board.toFEN();

                // 1) Graue Züge entfernen
                if (_selectedHistoryIndex < int(_history.size()) - 1) {
                    _history.resize(_selectedHistoryIndex + 1);
                }
                // 2) Neuen Zug anhängen
                _history.emplace_back(m, newFen, mover);
                // 3) Aktuellen Eintrag markieren
                _selectedHistoryIndex = int(_history.size()) - 1;

                // 4) Sieg­bedingungen prüfen VOR dem Side-Flip
                if (willCaptureGuard) {
                    _gameOver   = true;
                    _winMessage = (mover == ::Color::Red
                                ? "Rot hat den Wächter gefangen."
                                : "Blau hat den Wächter gefangen.");
                }
                else if (m.splitCount == 0) {
                    // Wächter‐Ziel­einzug?
                    if (mover == ::Color::Red && m.to == RED_GOAL_SQ) {
                        _winMessage = "Rot hat das Ziel erreicht.";
                        _gameOver   = true;
                    }
                    else if (mover == ::Color::Blue && m.to == BLUE_GOAL_SQ) {
                        _winMessage = "Blau hat das Ziel erreicht.";
                        _gameOver   = true;
                    }
                }

                // 5) Erst jetzt wechselst du die Seite
                _side = _board.sideToMove();
            }
            break;
        }

        _dragging=false;
        _legalMoves.clear();
        _dragMovedCount=0;
        _dragHoverTo=-1;

        if (_mode==Mode::HumanVsAI && !_gameOver && applied) {
            std::cout<<"[Event] Player move applied. Calling runAIMove(), new side="<<int(_side)<<std::endl;
            runAIMove();
        }

        // **Zusätzlich**: im manuellen Netzwerk-Modus den Zug abschicken
        if (_mode == Mode::Network && _networkManual && applied) {
            std::string mvStr = MoveGenerator::toString(appliedMove);
            if (std::count(mvStr.begin(), mvStr.end(), '-')==1) mvStr += "-1";
            _net->sendMove(mvStr);
            _net->receive();  // ACK konsumieren
        }
    }
}

void GamePage::update() {
    _fenField.update();


      if (_mode == Mode::Network) {
         // 1) Verbindung initial aufbauen
         if (!_net) {
             _net = std::make_unique<NetworkConnector>(_srvHost, _srvPort);
             if (!_net->connect()) {
                 std::cerr << "[GamePage] Network connect failed\n";
                 return;
             }
             _playerId = _net->playerId();
         }

         // 2) Status abfragen
         _net->sendGet();
         auto respOpt = _net->receive();
         if (!respOpt) return;
         auto& resp = *respOpt;
         if (!resp["bothConnected"].get<bool>()) return;

         // 3) Gegnerzug verarbeiten
         char turn = resp["turn"].get<std::string>()[0];
         std::string oldFen = _board.toFEN();
         std::string newFen = resp["board"].get<std::string>();
         if (oldFen != newFen) {
             ::Color lastMover = (turn=='r' ? ::Color::Blue : ::Color::Red);
             ::Board oldB, newB;
             oldB.loadFEN(oldFen);
             newB.loadFEN(newFen);
             Move opp = detectOpponentMove(oldB, newB, lastMover);
             if (!(opp.from==opp.to && opp.splitCount==0))
                 applyMoveLocally(opp, lastMover);
         }
         _board.loadFEN(newFen);

        // Im manuellen Modus: hier enden wir (kein AI-Zug)
        if (_networkManual)
            return;

         // 4) AI-Zug (Standardeinstellung)
         bool myTurn = (_playerId==0 && turn=='r') || (_playerId==1 && turn=='b');
         if (!myTurn) return;
         ::Color side = (turn=='r' ? ::Color::Red : ::Color::Blue);
         Move myMove = _networkAI
                       ? _networkAI->chooseMove(_board, side)
                       : ai::MinimaxAI(_networkAIDepth).chooseMove(_board, side);
         applyMoveLocally(myMove, side);

         // 5) Zug ans Server senden
         std::string mvStr = MoveGenerator::toString(myMove);
         if (std::count(mvStr.begin(), mvStr.end(), '-')==1) mvStr += "-1";
         _net->sendMove(mvStr);
         _net->receive();  // ACK
     }





    if (_mode == Mode::AIvsAI) {
        if (_gameOver) {
            // Wenn das Spiel schon vorbei ist, tun wir hier nichts mehr.
            // Dadurch werden keine weiteren AI-Züge generiert und
            // _winMessage bleibt bestehen, damit draw() es anzeigt.
            return;
        }

        // Wer zieht gerade?
        ::Color mover = _side;

        // Wähle die richtige AI (Rot = _redAI, Blau = _blueAI)
        std::unique_ptr<ai::AI>& currentAI =
            (mover == ::Color::Red ? _redAI : _blueAI);
        if (!currentAI) {
            // Eine Seite hat keine AI, also beenden wir das Spiel
            _gameOver = true;
            _winMessage = (_side == ::Color::Red
                ? "Blau hat gewonnen."
                : "Rot hat gewonnen.");
            return;
        }

        // 1) Zug berechnen (blockiert so lange, bis AlphaBeta fertig ist)
        Move mv = currentAI->chooseMove(_board, mover);
        if (mv.from < 0) {
            // AI konnte keinen gültigen Zug finden → Gegner gewinnt
            _gameOver = true;
            _winMessage = (mover == ::Color::Red
                ? "Blau hat gewonnen."
                : "Rot hat gewonnen.");
            return;
        }

        // 2) Prüfen, ob dabei ein Wächter geschlagen wird
        bool willCaptureGuard = _board.hasGuard(
            static_cast<::Color>(1 - int(mover)), mv.to
        );

        // 3) Zug ausführen
        _board.makeMove(mv);
        _fenField.setString(_board.toFEN());
        std::string newFen = _board.toFEN();

        // History aktualisieren
        if (_selectedHistoryIndex < int(_history.size()) - 1) {
            _history.resize(_selectedHistoryIndex + 1);
        }
        _history.emplace_back(mv, newFen, mover);
        _selectedHistoryIndex = int(_history.size()) - 1;

        // 4) Siegbedingungen prüfen VOR dem Side-Flip
        if (willCaptureGuard) {
            _gameOver = true;
            _winMessage = (mover == ::Color::Red
                ? "Rot hat den Wächter gefangen."
                : "Blau hat den Wächter gefangen.");
            return;
        }
        // 5) Sieg durch Erreichen des Ziels
        else if (mv.splitCount == 0) {
            // Nur wenn es ein Wächter-Zug war (splitCount == 0),
            // prüfen wir das Zielquadrat:
            if (mover == ::Color::Red && mv.to == RED_GOAL_SQ) {
                _gameOver   = true;
                _winMessage = "Rot hat das Ziel erreicht.";
                return;
            }
            else if (mover == ::Color::Blue && mv.to == BLUE_GOAL_SQ) {
                _gameOver   = true;
                _winMessage = "Blau hat das Ziel erreicht.";
                return;
            }
        }

        // 6) Falls das Board laut Board::isGameOver() schon vorbei ist,
        //    aber keiner der beiden oberen Fälle zutraf (z. B. Remis?),
        //    dann setzen wir einen generischen Gewinntext anhand von board.winner():
        if (_board.isGameOver()) {
            _gameOver = true;
            ::Color win = _board.winner();
            if (win == ::Color::Red) {
                _winMessage = "Rot hat gewonnen.";
            } else {
                _winMessage = "Blau hat gewonnen.";
            }
            return;
        }

        // 7) Immer noch hier: Seite wechseln, wenn das Spiel nicht aus ist
        _side = _board.sideToMove();
    }
}


} // namespace pages