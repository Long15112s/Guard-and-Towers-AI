// gui/pages/AIVsAIConfigPage.cpp

#include "AIVsAIConfigPage.hpp"
#include "AIRegistry.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

static const sf::Color TEXT_COLOR(240,240,240);

using namespace pages;

AIVsAIConfigPage::AIVsAIConfigPage(sf::RenderWindow& win, const sf::Font& font)
: _win(win)
, _font(font)
, _comboRed({150,30})
, _comboBlue({150,30})
, _btnDecRed({30,30})
, _btnIncRed({30,30})
, _btnDecBlue({30,30})
, _btnIncBlue({30,30})
, _btnStart({160,40})
{

  // statisches Hintergrundbild laden
  if (!_bgTexture.loadFromFile("assets/config.png")) {
      std::cerr << "Fehler: Konnte config.png nicht laden\n";
  } else {
      _bgSprite.setTexture(_bgTexture);
      auto ws = win.getSize();
      auto ts = _bgTexture.getSize();
      _bgSprite.setScale(float(ws.x)/ts.x, float(ws.y)/ts.y);
  }

 // Button-Textur laden
  if (!_buttonTexture.loadFromFile("assets/button.png")) {
      std::cerr << "Fehler: Konnte button.png nicht laden\n";
  }


    // VS‑Label
    _vsLabel.setFont(_font);
    _vsLabel.setCharacterSize(30);
    _vsLabel.setString("VS");
    _vsLabel.setFillColor(TEXT_COLOR);

    // KI‑Labels
    _labelRed.setFont(_font);
    _labelRed.setCharacterSize(24);
    _labelRed.setString("KI Rot");
    _labelRed.setFillColor(sf::Color::Red);

    _labelBlue = _labelRed;
    _labelBlue.setString("KI Blau");
    _labelBlue.setFillColor(sf::Color::Blue);

    // Depth‑Labels
    _labelDepthRed.setFont(_font);
    _labelDepthRed.setCharacterSize(16);
    _labelDepthRed.setString("Tiefe:");
    _labelDepthRed.setFillColor(TEXT_COLOR);

    _labelDepthBlue = _labelDepthRed;

    // Depth‑Values
    _txtDepthRed.setFont(_font);
    _txtDepthRed.setCharacterSize(16);
    _txtDepthRed.setFillColor(TEXT_COLOR);
    _txtDepthRed.setString(std::to_string(_depthValueRed));
    _txtDepthBlue = _txtDepthRed;

    // +/- Labels
    _lblDecRed.setFont(_font);
    _lblDecRed.setCharacterSize(20);
    _lblDecRed.setFillColor(TEXT_COLOR);
    _lblDecRed.setString("-");
    _lblIncRed = _lblDecRed;
    _lblIncRed.setString("+");

    _lblDecBlue = _lblDecRed;
    _lblIncBlue = _lblIncRed;

    // Start‑Button Label
    _lblStart.setFont(_font);
    _lblStart.setCharacterSize(18);
    _lblStart.setString("Los");
    _lblStart.setFillColor(TEXT_COLOR);

    // fill combos from global registry
    for (auto const& ai : ::AIRegistry()) {
        _comboRed.addItem(ai.name);
        _comboBlue.addItem(ai.name);
    }
    _comboRed.select(0);
    _comboBlue.select(0);
}

AIVsAIConfigPage::Action AIVsAIConfigPage::handleEvent(const sf::Event& ev) {
    _comboRed .handleEvent(ev, _win);
    _comboBlue.handleEvent(ev, _win);

    bool isRandR = (::AIRegistry()[_comboRed.selected()].name  == "ai::RandomAI");
    bool isRandB = (::AIRegistry()[_comboBlue.selected()].name == "ai::RandomAI");
    if (isRandR) _depthValueRed  = 0;
    if (isRandB) _depthValueBlue = 0;

    if (ev.type == sf::Event::MouseButtonReleased) {
        sf::Vector2f p = _win.mapPixelToCoords({ev.mouseButton.x, ev.mouseButton.y});
        if (_btnStart.contains(p))    return Action::Start;

        if (!isRandR) {
            if (_btnDecRed.contains(p)  && _depthValueRed>0) --_depthValueRed;
            if (_btnIncRed.contains(p)) ++_depthValueRed;
        }
        if (!isRandB) {
            if (_btnDecBlue.contains(p)  && _depthValueBlue>0) --_depthValueBlue;
            if (_btnIncBlue.contains(p)) ++_depthValueBlue;
        }
    }

    _txtDepthRed .setString(std::to_string(_depthValueRed));
    _txtDepthBlue.setString(std::to_string(_depthValueBlue));
    return Action::None;
}

void AIVsAIConfigPage::draw() {
  // 1) statisches Fullscreen-Background
  _win.clear();
  _win.draw(_bgSprite);

    // 3) schwebender VS-Text
    _vsTime += 0.025f;
    float yOff = std::sin(_vsTime)*8.f;
    float vsX  = _win.getSize().x/2.f - _vsLabel.getLocalBounds().width/2.f;
    _vsLabel.setPosition(vsX, 100.f + yOff);
    _win.draw(_vsLabel);

    bool isRandR = (::AIRegistry()[_comboRed.selected()].name  == "ai::RandomAI");
    bool isRandB = (::AIRegistry()[_comboBlue.selected()].name == "ai::RandomAI");

    float winW   = float(_win.getSize().x);
    const float comboW=150, comboH=30, gap=20, btnSep=80, spinY=190;
    const float numShift=15, labelShift=30, leftX=50, rightX=winW-200;

    // — KI Rot —
    _labelRed.setPosition(leftX,100);   _win.draw(_labelRed);
    _comboRed.setPosition({leftX,140}); _win.draw(_comboRed);
    float x = leftX + comboW + gap;
    _labelDepthRed.setPosition(x+labelShift,160); _win.draw(_labelDepthRed);

    // Depth‑Zahl immer zeichnen
    {
      auto n = _txtDepthRed.getLocalBounds();
      _txtDepthRed.setOrigin(n.left,n.top);
      _txtDepthRed.setPosition(x + (btnSep-n.width)/2 + numShift, spinY + (comboH-n.height)/2 + 0);
      _win.draw(_txtDepthRed);
    }
    // Buttons nur wenn nicht Random
    if (!isRandR) {
        _btnDecRed.setPosition({x,spinY}); _btnDecRed.box.setTexture(&_buttonTexture); _win.draw(_btnDecRed);
        { auto r=_btnDecRed.getGlobalBounds(), b=_lblDecRed.getLocalBounds();
          _lblDecRed.setOrigin(b.left,b.top);
          _lblDecRed.setPosition(r.left+(r.width-b.width)/2, r.top+(r.height-b.height)/2);
        } _win.draw(_lblDecRed);

        _btnIncRed.setPosition({x+btnSep,spinY}); _btnIncRed.box.setTexture(&_buttonTexture); _win.draw(_btnIncRed);
        { auto r=_btnIncRed.getGlobalBounds(), b=_lblIncRed.getLocalBounds();
          _lblIncRed.setOrigin(b.left,b.top);
          _lblIncRed.setPosition(r.left+(r.width-b.width)/2, r.top+(r.height-b.height)/2);
        } _win.draw(_lblIncRed);
    }

    // — KI Blau —
    _labelBlue.setPosition(rightX,100);   _win.draw(_labelBlue);
    _comboBlue.setPosition({rightX,140}); _win.draw(_comboBlue);
    float x2 = rightX - gap - comboH - 75;  // blueShift
    _labelDepthBlue.setPosition(x2+labelShift,160); _win.draw(_labelDepthBlue);

    // Depth‑Zahl immer zeichnen
    {
      auto n = _txtDepthBlue.getLocalBounds();
      _txtDepthBlue.setOrigin(n.left,n.top);
      _txtDepthBlue.setPosition(x2 + (btnSep-n.width)/2 + numShift, spinY + (comboH-n.height)/2 + 0);
      _win.draw(_txtDepthBlue);
    }
    if (!isRandB) {
        _btnDecBlue.setPosition({x2,spinY}); _btnDecBlue.box.setTexture(&_buttonTexture); _win.draw(_btnDecBlue);
        { auto r=_btnDecBlue.getGlobalBounds(), b=_lblDecBlue.getLocalBounds();
          _lblDecBlue.setOrigin(b.left,b.top);
          _lblDecBlue.setPosition(r.left+(r.width-b.width)/2, r.top+(r.height-b.height)/2);
        } _win.draw(_lblDecBlue);

        _btnIncBlue.setPosition({x2+btnSep,spinY}); _btnIncBlue.box.setTexture(&_buttonTexture); _win.draw(_btnIncBlue);
        { auto r=_btnIncBlue.getGlobalBounds(), b=_lblIncBlue.getLocalBounds();
          _lblIncBlue.setOrigin(b.left,b.top);
          _lblIncBlue.setPosition(r.left+(r.width-b.width)/2, r.top+(r.height-b.height)/2);
        } _win.draw(_lblIncBlue);
    }

    // — Los‑Button —
    float sx = winW/2 - _btnStart.getSize().x/2;
    _btnStart.setPosition({sx,260}); _btnStart.box.setTexture(&_buttonTexture); _win.draw(_btnStart);
    { auto r=_btnStart.getGlobalBounds(), b=_lblStart.getLocalBounds();
      _lblStart.setOrigin(b.left,b.top);
      _lblStart.setPosition(r.left+(r.width-b.width)/2, r.top+(r.height-b.height)/2);
    } _win.draw(_lblStart);

    _win.display();
}
