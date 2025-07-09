#include "PvEConfigPage.hpp"
#include "AIRegistry.hpp"
#include <string>
#include <cmath>
#include <iostream>

using sf::RenderWindow;
using sf::Font;
using sf::Text;
using sf::Vector2f;

static const sf::Color TEXT_COLOR(240,240,240);

using namespace sf;

namespace pages {

PvEConfigPage::PvEConfigPage(RenderWindow& win, Font& font)
: _win(win), _font(font), _combo({150,30}),
  _btnSwap({160,40}), _btnStart({160,40}),
  _btnDepthDec({30,30}), _btnDepthInc({30,30}),
  _pulseTime(0.f), _vsTime(0.f)
  
{   
    // statisches Hintergrundbild laden
    if (!_bgTexture.loadFromFile("assets/config.png")) {
        std::cerr << "Fehler: Konnte config.png nicht laden\n";
    } else {
        _bgSprite.setTexture(_bgTexture);
        auto ws = win.getSize();
        auto ts = _bgTexture.getSize();
        _bgSprite.setScale(
            float(ws.x)/float(ts.x),
            float(ws.y)/float(ts.y)
        );
    }

    // Button-Textur laden
    if (!_buttonTexture.loadFromFile("assets/button.png")) {
        std::cerr << "Fehler: Konnte button.png nicht laden\n";
    }




    _vs.setFont(font);
    _vs.setCharacterSize(30);
    _vs.setString("VS");
    _vs.setFillColor(TEXT_COLOR);

    _combo.setPosition({win.getSize().x / 2.f + 60.f, 100.f});
    for (auto const& ai : AIRegistry())
        _combo.addItem(ai.name);
    _combo.select(0);

    if (AIRegistry()[_combo.selected()].name == "ai::RandomAI")
        _depthValue = 0;

    _btnDepthDec.setPosition({0, 0});
    _btnDepthInc.setPosition({0, 0});

    _labelDepth.setFont(font);
    _labelDepth.setCharacterSize(20);
    _labelDepth.setFillColor(TEXT_COLOR);
    _labelDepth.setString("Tiefe:");

    _txtDepth.setFont(font);
    _txtDepth.setCharacterSize(20);
    _txtDepth.setFillColor(TEXT_COLOR);
    _txtDepth.setString(std::to_string(_depthValue));

    _lblDec.setFont(font);
    _lblDec.setCharacterSize(20);
    _lblDec.setFillColor(TEXT_COLOR);
    _lblDec.setString("-");

    _lblInc.setFont(font);
    _lblInc.setCharacterSize(20);
    _lblInc.setFillColor(TEXT_COLOR);
    _lblInc.setString("+");

    _lblSwap = Text("Swap", font,18);
    _lblSwap.setFillColor(TEXT_COLOR);

    _lblStart = Text("Los", font, 20);
    _lblStart.setFillColor(TEXT_COLOR);

    _labelLeft.setFont(font);   _labelLeft.setCharacterSize(24);
    _labelRight.setFont(font);  _labelRight.setCharacterSize(24);
}

PvEConfigPage::Action PvEConfigPage::handleEvent(const Event& ev) {
    _combo.handleEvent(ev, _win);
    bool isRandom = AIRegistry()[_combo.selected()].name == "ai::RandomAI";
    if (isRandom) {
        _depthValue = 0;
    }

    if (ev.type == Event::MouseButtonReleased) {
        Vector2f p = _win.mapPixelToCoords({ev.mouseButton.x, ev.mouseButton.y});
        if (_btnSwap.getGlobalBounds().contains(p)) {
            _playerRed = !_playerRed;
            return Action::None;
        }
        if (_btnStart.getGlobalBounds().contains(p)) {
            return Action::Start;
        }
        if (!isRandom) {
            if (_btnDepthDec.getGlobalBounds().contains(p) && _depthValue > 0)
                --_depthValue;
            if (_btnDepthInc.getGlobalBounds().contains(p))
                ++_depthValue;
        }
    }

    _txtDepth.setString(std::to_string(_depthValue));
    return Action::None;
}

void PvEConfigPage::draw() {
    // 1) statisches Hintergrundbild
    _win.clear();
    _win.draw(_bgSprite);

    // 3) schwebender VS-Text
    _vsTime += 0.025f;
    float yOff = sin(_vsTime)*8.f;
    float vx = _win.getSize().x/2.f - _vs.getLocalBounds().width/2;
    _vs.setPosition(vx, 100.f + yOff);
    _win.draw(_vs);

    if (_playerRed) {
        _labelLeft.setString("Spieler Rot");
        _labelLeft.setFillColor(sf::Color::Red);
        _labelRight.setString("KI Blau");
        _labelRight.setFillColor(sf::Color::Blue);
    } else {
        _labelLeft.setString("KI Rot");
        _labelLeft.setFillColor(sf::Color::Red);
        _labelRight.setString("Spieler Blau");
        _labelRight.setFillColor(sf::Color::Blue);
    }
    _labelLeft.setPosition(50.f, 100.f);
    _labelRight.setPosition(_win.getSize().x - 200.f, 100.f);
    _win.draw(_labelLeft);
    _win.draw(_labelRight);

    float vsX = (_win.getSize().x - _vs.getLocalBounds().width) / 2.f;
    _vs.setPosition(vsX, 100.f);
    _win.draw(_vs);

    const float comboW = 150.f, comboH = 30.f;
    sf::Text& aiLabel = _playerRed ? _labelRight : _labelLeft;
    auto labelPos = aiLabel.getPosition();
    auto labelBounds = aiLabel.getLocalBounds();
    const float bigOffset = 50.f;
    const float leftShift = 120.f;
    float comboX = labelPos.x + labelBounds.width/2.f - comboW/2.f - leftShift;
    float comboY = labelPos.y + labelBounds.height + bigOffset;
    float extraX = _playerRed ? 0.f : 150.f;
    comboX += extraX;

    _combo.setPosition({ comboX, comboY });
    _win.draw(_combo);

    bool isRandom = AIRegistry()[ _combo.selected() ].name == "ai::RandomAI";

    const float gap = 50.f;
    float depthX = comboX + comboW + gap;
    float depthY = comboY;

    _labelDepth.setPosition(depthX + 25.f, depthY - _labelDepth.getCharacterSize() - 5.f);
    _win.draw(_labelDepth);
    _txtDepth.setPosition(depthX + _btnDepthDec.getSize().x + 20.f, depthY + 2);
    _win.draw(_txtDepth);

    if (!isRandom) {
        _btnDepthDec.setPosition({ depthX, depthY });
        _btnDepthDec.box.setTexture(&_buttonTexture);
        _win.draw(_btnDepthDec);

        auto b = _lblDec.getLocalBounds();
        _lblDec.setPosition(
            depthX + (_btnDepthDec.getSize().x - b.width)/2 - b.left,
            depthY + (_btnDepthDec.getSize().y - b.height)/2 - b.top
        );
        _win.draw(_lblDec);

        float incX = depthX + 80;
        _btnDepthInc.setPosition({ incX, depthY });
        _btnDepthInc.box.setTexture(&_buttonTexture);
        _win.draw(_btnDepthInc);

        b = _lblInc.getLocalBounds();
        _lblInc.setPosition(
            incX + (_btnDepthInc.getSize().x - b.width)/2 - b.left,
            depthY + (_btnDepthInc.getSize().y - b.height)/2 - b.top
        );
        _win.draw(_lblInc);
    }

    const float btnGapY = 40.f;
    float buttonsY = comboY + comboH + btnGapY;
    float btnX     = (_win.getSize().x - _btnSwap.getSize().x) / 2.f;

    _btnSwap.setPosition({ btnX, buttonsY });
    _btnSwap.box.setTexture(&_buttonTexture);
    {
      auto b = _lblSwap.getLocalBounds();
      _lblSwap.setPosition(
        btnX + (_btnSwap.getSize().x - b.width)/2 - b.left,
        buttonsY + (_btnSwap.getSize().y - b.height)/2 - b.top
      );
    }
    _win.draw(_btnSwap);
    _win.draw(_lblSwap);

    float startY = buttonsY + _btnSwap.getSize().y + btnGapY;
    _btnStart.setPosition({ btnX, startY });
    _btnStart.box.setTexture(&_buttonTexture);
    {
      auto b = _lblStart.getLocalBounds();
      _lblStart.setPosition(
        btnX + (_btnStart.getSize().x - b.width)/2 - b.left,
        startY + (_btnStart.getSize().y - b.height)/2 - b.top
      );
    }
    _win.draw(_btnStart);
    _win.draw(_lblStart);

    _win.display();
}

bool PvEConfigPage::playerIsRed() const {
    return _playerRed;
}

std::string PvEConfigPage::selectedAIModel() const {
    return AIRegistry()[ _combo.selected() ].name;
}

int PvEConfigPage::selected() const {
    return _combo.selected();
}

int PvEConfigPage::selectedDepth() const {
    if (AIRegistry()[_combo.selected()].name == "RandomAI")
        return 0;
    return _depthValue;
}

} // namespace pages
