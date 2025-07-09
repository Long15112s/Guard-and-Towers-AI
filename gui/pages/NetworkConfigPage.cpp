// gui/pages/NetworkConfigPage.cpp
#include "NetworkConfigPage.hpp"
#include "ai/AIRegistry.hpp"
#include <iostream>

using namespace pages;
static const sf::Color TEXT_COLOR(240,240,240);

NetworkConfigPage::NetworkConfigPage(sf::RenderWindow& win, const sf::Font& font)
: _win(win), _font(font)
, _combo({150,30})
, _btnDepthDec({30,30}), _btnDepthInc({30,30})
, _btnStart({200,40}), _btnManual({200,40}), _btnBack({200,40})
{
    // Hintergrund
    if (!_bgTex.loadFromFile("assets/config.png"))
        std::cerr<<"Could not load config.png\n";
    _bgSpr.setTexture(_bgTex);
    auto ws = _win.getSize();
    _bgSpr.setScale(
      float(ws.x)/_bgTex.getSize().x,
      float(ws.y)/_bgTex.getSize().y
    );

    // Button-Textur
    if (!_buttonTexture.loadFromFile("assets/button.png"))
        std::cerr<<"Could not load button.png\n";
    for (auto *b : {&_btnDepthDec, &_btnDepthInc, &_btnStart, &_btnManual, &_btnBack})
        b->box.setTexture(&_buttonTexture);

    // Titel
    _lblTitle = sf::Text("Online Game Config", font, 32);
    _lblTitle.setFillColor(TEXT_COLOR);
    auto tb = _lblTitle.getLocalBounds();
    _lblTitle.setOrigin(tb.left+tb.width/2, tb.top+tb.height/2);
    _lblTitle.setPosition(ws.x/2.f,  60.f);

    // Combo und Tiefe nebeneinander
    float baseY = 160.f;
    float centerX = ws.x/2.f;
    _combo.setPosition({centerX - 300.f, baseY});
    for (auto const& ai : AIRegistry())
        _combo.addItem(ai.name);
    _combo.select(0);

    _lblDepth = sf::Text("Tiefe:", font,24);
    _lblDepth.setFillColor(TEXT_COLOR);
    _lblDepth.setPosition({centerX - 100.f, baseY + 5.f});

    _btnDepthDec.setPosition({centerX - 30.f, baseY});
    _btnDepthInc.setPosition({centerX + 10.f, baseY});
    _lblDec = sf::Text("-", font,20); _lblDec.setFillColor(TEXT_COLOR);
    _lblInc = sf::Text("+", font,20); _lblInc.setFillColor(TEXT_COLOR);

    // depth value text
    _txtDepth = sf::Text(std::to_string(_depthValue), font, 24);
    _txtDepth.setFillColor(TEXT_COLOR);

    auto centerLbl = [&](ui::Button& b, sf::Text& lbl){
      auto bb = b.getGlobalBounds(), lb = lbl.getLocalBounds();
      lbl.setPosition(
        bb.left + (bb.width - lb.width)/2 - lb.left,
        bb.top  + (bb.height - lb.height)/2 - lb.top
      );
    };
    centerLbl(_btnDepthDec, _lblDec);
    centerLbl(_btnDepthInc, _lblInc);

    // Action-Buttons vertikal
    float btnX = centerX - 100.f;
    float btnY = baseY + 100.f;
    float gap  = 60.f;

    _btnStart .setPosition({btnX,          btnY});
    _btnManual.setPosition({btnX,          btnY + gap});
    _btnBack  .setPosition({btnX,          btnY + 2*gap});

    _lblStart  = sf::Text("Start AI", font,20);    _lblStart .setFillColor(TEXT_COLOR);
    _lblManual = sf::Text("Play Manual", font,20); _lblManual.setFillColor(TEXT_COLOR);
    _lblBack   = sf::Text("Back", font,20);        _lblBack  .setFillColor(TEXT_COLOR);

    centerLbl(_btnStart,  _lblStart);
    centerLbl(_btnManual, _lblManual);
    centerLbl(_btnBack,   _lblBack);
}

NetworkConfigPage::Action NetworkConfigPage::handleEvent(const sf::Event& ev) {
    _combo.handleEvent(ev, _win);

    if (ev.type==sf::Event::MouseButtonReleased) {
        auto p = _win.mapPixelToCoords({ev.mouseButton.x, ev.mouseButton.y});
        // Tiefe
        if (_btnDepthDec .getGlobalBounds().contains(p) && _depthValue>0) --_depthValue;
        if (_btnDepthInc .getGlobalBounds().contains(p))                  ++_depthValue;
        _txtDepth.setString(std::to_string(_depthValue));

        // Buttons
        if (_btnStart .getGlobalBounds().contains(p)) { _manualSelected = false; return Action::StartAI;    }
        if (_btnManual.getGlobalBounds().contains(p)) { _manualSelected = true;  return Action::StartManual; }
        if (_btnBack  .getGlobalBounds().contains(p))                   return Action::Back;
    }
    return Action::None;
}

void NetworkConfigPage::draw() {
    _win.clear({30,30,40});
    _win.draw(_bgSpr);
    _win.draw(_lblTitle);

    // Combo
    _win.draw(_combo);

    // Tiefe + Buttons
    _win.draw(_lblDepth);
    _win.draw(_btnDepthDec); _win.draw(_lblDec);
    _win.draw(_btnDepthInc); _win.draw(_lblInc);

    // Tiefe-Wert in der Mitte
    auto dB = _btnDepthDec.getGlobalBounds(), iB = _btnDepthInc.getGlobalBounds();
    auto tb = _txtDepth.getLocalBounds();
    _txtDepth.setOrigin(tb.left + tb.width/2, tb.top + tb.height/2);
    float x = dB.left + dB.width + (iB.left - (dB.left + dB.width))*0.5f;
    float y = dB.top + dB.height/2;
    _txtDepth.setPosition({x,y});
    _win.draw(_txtDepth);

    // Vertikale Buttons
    _win.draw(_btnStart);  _win.draw(_lblStart);
    _win.draw(_btnManual); _win.draw(_lblManual);
    _win.draw(_btnBack);   _win.draw(_lblBack);

    _win.display();
}
