// gui/pages/StartPage.cpp
#include "StartPage.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace sf;
using namespace pages;

static constexpr float BTN_W    = 300.f;
static constexpr float BTN_H    = 50.f;
static constexpr float BTN_GAP  = 20.f;
static const Color    COL_BG1   (30, 30, 40);
static const Color    COL_BG2   (50, 50, 70);
static const Color    COL_TEXT  (240,240,240);
static const Color    COL_SHADOW(20,20,20,150);

StartPage::StartPage(RenderWindow& win, const Font& font, float top)
: _win(win)
, _font(font)
, _top(top)
, _pulseTime(0.f)
, _titleTime(0.f)
{
    // statisches Startbild laden
    if (!_bgTexture.loadFromFile("assets/Startscreen.png")) {
        std::cerr << "Fehler: Konnte startscreen.png nicht laden\n";
    } else {
        _bgSprite.setTexture(_bgTexture);
        // falls nötig: auf Fenstergröße skalieren
        auto ws = win.getSize();
        auto ts = _bgTexture.getSize();
        _bgSprite.setScale(
            float(ws.x)/float(ts.x),
            float(ws.y)/float(ts.y)
        );
    }

    // --- mittelalterliche Titel-Schrift laden ---
    if (!_titleFont.loadFromFile("assets/MedievalSharp-Regular.ttf")) {
        std::cerr << "Fehler: Konnte MedievalSharp-Regular.ttf nicht laden\n";
    }


    // Button-Textur laden
    if (!_buttonTexture.loadFromFile("assets/button.png")) {
        std::cerr << "Fehler: Konnte button.png nicht laden\n";
    }




    // Buttons erzeugen
    float cx = win.getSize().x/2.f - BTN_W/2.f;
    float y = _top + 120.f;
    _btnPvP   = ui::makeButton(_font, "Spieler vs Spieler", {cx, y}, {BTN_W, BTN_H});
    _btnPvP.box.setTexture(&_buttonTexture);
    y += BTN_H + BTN_GAP + 20.f;
    _btnPvE   = ui::makeButton(_font, "Spieler vs KI",      {cx, y}, {BTN_W, BTN_H});
    _btnPvE.box.setTexture(&_buttonTexture);
    y += BTN_H + BTN_GAP + 20.f;
    _btnAIvAI = ui::makeButton(_font, "KI vs KI",           {cx, y}, {BTN_W, BTN_H});
    _btnAIvAI.box.setTexture(&_buttonTexture);
    y += BTN_H + BTN_GAP + 20.f;
    _btnOnline = ui::makeButton(_font, "Online spielen", {cx, y}, {BTN_W, BTN_H});
    _btnOnline.box.setTexture(&_buttonTexture);
    y += BTN_H + BTN_GAP + 100.f;
    _btnExit  = ui::makeButton(_font, "Beenden",            {cx, y}, {BTN_W, BTN_H});
    _btnExit.box.setTexture(&_buttonTexture);


    // Titel + Schatten
    _title.setFont(_titleFont);
    _title.setString("Guard & Towers");
    _title.setCharacterSize(48);
    _title.setFillColor(COL_TEXT);
    // Zentriere Titel
    {
        auto tb = _title.getLocalBounds();
        _title.setOrigin(tb.left + tb.width/2.f,
                         tb.top  + tb.height/2.f);
        // vertikal etwas unterhalb von „top“, z.B. top + 60
        _title.setPosition(win.getSize().x/2.f,
                           _top + 60.f);
    }
     _titleShadow = _title;
     _titleShadow.setFont(_titleFont);
     _titleShadow.setFillColor(COL_SHADOW);
    // Zentriere Schatten mit kleinem Versatz
    {
        _titleShadow.setOrigin(_title.getOrigin());
        _titleShadow.setPosition(
            _title.getPosition().x + 2.f,
            _title.getPosition().y + 2.f
        );
    }

}

StartPage::Action StartPage::handleEvent(const Event& ev) {
    if (ev.type == Event::MouseButtonReleased && ev.mouseButton.button == Mouse::Left) {
        Vector2f p = _win.mapPixelToCoords({ev.mouseButton.x, ev.mouseButton.y});
        if (_btnPvP.contains(p))   return _action = Action::PvP;
        if (_btnPvE.contains(p))   return _action = Action::PvE;
        if (_btnAIvAI.contains(p)) return _action = Action::AIvAI;
        if (_btnOnline.contains(p)) return _action = Action::Online;
        if (_btnExit.contains(p))  return _action = Action::Exit;
        
    }
    return Action::None;
}

void StartPage::draw() {
    _win.clear();
    _win.draw(_bgSprite);

    // 3) Titel
    _win.draw(_titleShadow);
    _win.draw(_title);

    // 4) Buttons
    _win.draw(_btnPvP);
    _win.draw(_btnPvE);
    _win.draw(_btnAIvAI);
    _win.draw(_btnOnline);
    _win.draw(_btnExit);
}
