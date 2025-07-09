#include "GamePage.hpp"
#include <sstream>
#include <cmath>  // for std::max, std::clamp if needed

using namespace sf;
using namespace std;

namespace pages {

// Höhe einer History-Zeile (zwei Zeilen: Move + FEN)
static constexpr float ENTRY_H = GuiStyle::HISTORY_LINE_H * 2;
static constexpr float PANEL_OFFSET_X = 50.f;
static constexpr float BG_OFFSET_X = 90.f;

void GamePage::draw() {
    // 1) Clear und dann Full-Screen Holz-Hintergrund
    _win.clear();       

        auto ws = _win.getSize();
        float w  = float(ws.x);

        // 1) Haupt‐Sprite rechts verschoben
        _fullBgSpr.setPosition(BG_OFFSET_X, 0.f);
        _win.draw(_fullBgSpr);

        // 2) Zweite Kopie links daneben, damit keine Lücke entsteht
        _fullBgSpr.setPosition(BG_OFFSET_X - w, 0.f);
        _win.draw(_fullBgSpr);

        // (Danach wieder Position fürs Frame zurücksetzen, falls Du die Sprites weiter verwendest)
    _fullBgSpr.setPosition(BG_OFFSET_X, 0.f);

    _frameShape.setOutlineColor(sf::Color::Black);
    _frameShape.setOutlineThickness(2.f);
    _win.draw(_frameShape);
    _win.draw(_frameSpr);

    // 2) Win‑Message (falls Spiel vorbei)
    if (_gameOver && !_winMessage.empty()) {
        sf::String utf8 = sf::String::fromUtf8(
            _winMessage.begin(), _winMessage.end()
        );
        sf::Text winTxt(utf8, _font, 24);
        winTxt.setFillColor(GuiStyle::TEXT);

        float boardW = 7 * _TILE;
        float boardX = _M_L;
        auto tb = winTxt.getLocalBounds();
        float x = boardX + (boardW - tb.width) * 0.5f - tb.left;
        float y = _M_T - tb.height - tb.top - 10.f;
        x = std::clamp(x, 5.f, float(_win.getSize().x) - tb.width - 5.f);
        y = std::max(5.f, y);

        winTxt.setPosition(x, y + 7.f);
        _win.draw(winTxt);
    }

    // 3) Holz-Hintergrund für das 7×7-Brett
    _win.draw(_boardBackground);

    for (int y = 0; y < 7; ++y) {
        for (int x = 0; x < 7; ++x) {
            sf::RectangleShape cell({ _TILE - 2.f, _TILE - 2.f });
            cell.setPosition(_M_L + x*_TILE + 1.f,
                             _M_T + y*_TILE + 1.f);
            // Füllung transparent, damit Holz durchscheint
            cell.setFillColor(sf::Color::Transparent);
            // gelbe Markierung für D7/D1
            if ((y==0 || y==6) && x==3)
                cell.setFillColor(sf::Color(255, 255, 0, 50));
            cell.setOutlineColor(sf::Color::Black);
            cell.setOutlineThickness(1.f);
            _win.draw(cell);
        }
    }

    // 4) Koordinaten (links und unten)
    for (int y = 0; y < 7; ++y) {
        sf::Text t(std::to_string(7 - y), _font, 18);
        t.setFillColor(GuiStyle::TEXT);
        auto lb = t.getLocalBounds();
        // x-Position: etwas unter deinem Frame-Sprite
        t.setPosition(
            _frameSpr.getPosition().x + 15.f,
            _frameSpr.getPosition().y + y*_TILE + _TILE/2.f
              - lb.height/2.f - lb.top + 35.f
        );
        _win.draw(t);
    }
    for (int x = 0; x < 7; ++x) {
        sf::Text t(std::string(1, char('A'+x)), _font, 18);
        t.setFillColor(GuiStyle::TEXT);
        auto lb = t.getLocalBounds();
        // y-Position: etwas unter deinem Frame-Sprite
        t.setPosition(
            _frameSpr.getPosition().x + x*_TILE + _TILE/2.f
              - lb.width/2.f - lb.left + 40.f,
            _frameSpr.getPosition().y + 7*_TILE + 40.f
        );
        _win.draw(t);
    }

    // --- Drag-Preview mit Textur statt FillColor ---
    if (_dragging) {
        int us = int(_side), them = 1 - us;
        for (auto &m : _legalMoves) {
            sf::Vector2f pos = {
                _M_L + (m.to % 7)*_TILE,
                _M_T + (6 - m.to/7)*_TILE
            };
            bool guardCap = _board.hasGuard(static_cast<::Color>(them), m.to);
            bool stackCap = _board.hasStack(static_cast<::Color>(them), m.to);
            bool ownStack = (m.splitCount>0)
                         && _board.hasStack(static_cast<::Color>(us), m.to);

            if (guardCap) {
                sf::RectangleShape hl(sf::Vector2f(_TILE, _TILE));
                hl.setPosition(pos);
                hl.setFillColor({0,255,0,100});
                _win.draw(hl);
            } else {
                sf::CircleShape hl(_TILE/2.f - 4.f);
                hl.setOrigin(hl.getRadius(), hl.getRadius());
                hl.setPosition(pos.x+_TILE/2.f, pos.y+_TILE/2.f);
                if (stackCap||ownStack) {
                    hl.setFillColor(sf::Color::Transparent);
                    hl.setOutlineColor({0,255,0,100});
                    hl.setOutlineThickness(4.f);
                } else {
                    hl.setFillColor({0,255,0,100});
                    hl.setOutlineThickness(0);
                }
                _win.draw(hl);
            }
        }
    }

    // 6) Figuren zeichnen
    for (int sq = 0; sq < 49; ++sq) {
        bool isRG = _board.hasGuard(::Color::Red,   sq);
        bool isBG = _board.hasGuard(::Color::Blue,  sq);
        bool isRS = _board.hasStack(::Color::Red,   sq);
        bool isBS = _board.hasStack(::Color::Blue,  sq);
        if (!(isRG||isBG||isRS||isBS)) continue;

        sf::Vector2f p = sqToPos(sq);
        sf::Color col = (isRG||isRS) ? sf::Color::Red : sf::Color::Blue;

        if (isRG||isBG) {
            sf::RectangleShape g(sf::Vector2f(_TILE-10.f, _TILE-10.f));
            g.setOrigin(g.getSize().x/2.f, g.getSize().y/2.f);
            g.setPosition(p);

            // Textur statt Farbfüllung
            if (col == sf::Color::Red)
                g.setTexture(&_redTexture);
            else
                g.setTexture(&_blueTexture);

            g.setOutlineColor(sf::Color::Black);
            g.setOutlineThickness(2.f);
            _win.draw(g);
        }
        if (isRS||isBS) {
            sf::CircleShape s(_TILE/2.f - 4.f);
            s.setOrigin(s.getRadius(), s.getRadius());
            s.setPosition(p);

            // Textur statt Farbfüllung
            if (col == sf::Color::Red)
                s.setTexture(&_redTexture);
            else
                s.setTexture(&_blueTexture);

            s.setOutlineColor(sf::Color::Black);
            s.setOutlineThickness(2.f);
            _win.draw(s);

            int h = _board.heightAt(sq);
            if (h > 0) {
                sf::Text ht(std::to_string(h), _font, 18);
                ht.setFillColor(GuiStyle::TEXT);
                auto lb = ht.getLocalBounds();
                ht.setPosition(
                  p.x - lb.width/2.f - lb.left,
                  p.y - lb.height/2.f - lb.top
                );
                _win.draw(ht);
            }
        }
    }

    // 7) Rest‑Stack beim Draggen
    if (_dragging && !_dragIsGuard && _dragHoverTo >= 0) {
        int from = _fromSq;
        int origH = _board.heightAt(from);
        int moved = _dragMovedCount;
        int remH  = origH - moved;

        sf::CircleShape rem(_TILE/2.f - 4.f);
        rem.setOrigin(rem.getRadius(), rem.getRadius());
        rem.setPosition(sqToPos(from));
        // Textur statt einfärben
        if (_side == ::Color::Red)
            rem.setTexture(&_redTexture);
        else
            rem.setTexture(&_blueTexture);

        rem.setOutlineColor(sf::Color::Black);
        rem.setOutlineThickness(2.f);
        _win.draw(rem);

        sf::Text rt(std::to_string(remH), _font, 18);
        rt.setFillColor(GuiStyle::TEXT);
        auto lb = rt.getLocalBounds();
        rt.setPosition(
          rem.getPosition().x - lb.width/2.f - lb.left,
          rem.getPosition().y - lb.height/2.f - lb.top
        );
        _win.draw(rt);
    }

    if (_dragging) {
        if (_dragIsGuard) {
            // Guard-Drag: Textur anhand der Seite wählen
            if (_side == ::Color::Red)
                _dragRect.setTexture(&_redTexture);
            else
                _dragRect.setTexture(&_blueTexture);
            _win.draw(_dragRect);
        }
        else {
            // Stack-Drag: Textur anhand der Seite wählen
            if (_side == ::Color::Red)
                _dragShape.setTexture(&_redTexture);
            else
                _dragShape.setTexture(&_blueTexture);
            _win.draw(_dragShape);
            _win.draw(_dragText);
        }
    }


    // 9) FEN‑Field & Load‑Button
    _fenField.draw(_win);
    _win.draw(_loadBtn);
    _win.draw(_loadLabel);

    // 10) History‑Panel Hintergrund
    float histX  = _M_L + 7*_TILE + GuiStyle::PANEL_PADDING + PANEL_OFFSET_X + 50.f;
    float histY  = _M_T + 70.f;
    float panelW = _win.getSize().x - histX - GuiStyle::PANEL_PADDING;
    float panelH = 6*_TILE;
    // MoveHistory-Panel ohne Hintergrund
    sf::RectangleShape panelBg(sf::Vector2f(panelW, panelH));
    panelBg.setPosition(histX, histY);
    // transparent zeichnen (oder das PanelBg ganz weglassen)
    panelBg.setFillColor(sf::Color::Transparent);
    _win.draw(panelBg);


    // 11) Mode‑Label oben rechts über dem Spielbrett zeichnen
    auto lb = _modeLabel.getLocalBounds();
    _modeLabel.setOrigin(
        lb.left + lb.width/2.f - 30.f,
        lb.top  + lb.height/2.f
    );

    float centerX = _M_L + 7*_TILE/2.f + 440.f + PANEL_OFFSET_X;
    float centerY = _M_T - 20.f;
    _modeLabel.setPosition(centerX, centerY);

    // 4) Zeichnen
    _win.draw(_modeLabel);


    // 12) History‑Einträge
    for (size_t i = 0; i < _history.size(); ++i) {
        float y = histY + GuiStyle::HISTORY_PADDING + i*ENTRY_H - _historyScrollOffset;
        if (y < histY || y > histY + panelH - ENTRY_H) continue;

        // Move‑Text
        std::ostringstream ss;
        ss << (i==0 ? "Startpos"
                   : std::to_string(i)+". "+ MoveGenerator::toString(_history[i].move));
        sf::Text txt(ss.str(), _font, 14);
        if (i == 0) {
            txt.setFillColor(GuiStyle::TEXT);
        } else if (i <= _selectedHistoryIndex) {
            txt.setFillColor(
                _history[i].mover==::Color::Red ? sf::Color::Red
                                                : sf::Color::Blue
            );
        } else {
            txt.setFillColor(sf::Color(150,150,150));
        }
        txt.setPosition(histX + GuiStyle::HISTORY_PADDING + 5.f, y);
        _win.draw(txt);

        // FEN‑String unterhalb des Moves, nur für Einträge >0
        if (i > 0) {
            sf::Text fenTxt(_history[i].fen, _font, 8);
            // FEN-Strings in Spielerfarbe (Rot oder Blau)
            if (_history[i].mover == ::Color::Red)
                fenTxt.setFillColor(sf::Color::Red);
            else
                fenTxt.setFillColor(sf::Color::Blue);
            float fx = histX + GuiStyle::HISTORY_PADDING;
            float fy = y + GuiStyle::HISTORY_LINE_H;
            fenTxt.setPosition(fx, fy);
            _win.draw(fenTxt);
        }
    }

    // 13) NewGame / Back‑Buttons
    _win.draw(_newGameBtn);
    _win.draw(_newGameLabel);
    _win.draw(_backBtn);
    _win.draw(_backLabel);

    // 14) Frame abschließen
    _win.display();
}

} // namespace pages
