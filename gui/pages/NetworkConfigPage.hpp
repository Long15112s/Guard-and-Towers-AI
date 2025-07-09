// gui/pages/NetworkConfigPage.hpp
#pragma once

#include <SFML/Graphics.hpp>
#include "../ui/ComboBox.hpp"
#include "../ui/Button.hpp"
#include "ai/AIRegistry.hpp"
#include <string>

namespace pages {

class NetworkConfigPage {
public:
    enum class Action { None, StartAI, StartManual, Back };

    NetworkConfigPage(sf::RenderWindow& win, const sf::Font& font);
    Action handleEvent(const sf::Event& ev);
    void   draw();

    bool        playerIsRed()     const { return _playerRed; }
    int         selected()        const { return _combo.selected(); }
    int         selectedDepth()   const {
                     return AIRegistry()[_combo.selected()].name=="RandomAI"
                            ? 0
                            : _depthValue;
                 }
    std::string selectedAIModel() const {
                     return AIRegistry()[_combo.selected()].name;
                 }
    std::string serverHost()      const { return _srvHost; }
    unsigned    serverPort()      const { return _srvPort; }
    bool        playManual()      const { return _manualSelected; }

private:
    sf::RenderWindow& _win;
    const sf::Font&   _font;

    ui::ComboBox      _combo;
    ui::Button        _btnDepthDec, _btnDepthInc;
    ui::Button        _btnStart, _btnManual, _btnBack;

    sf::Texture       _bgTex, _buttonTexture;
    sf::Sprite        _bgSpr;

    sf::Text          _lblTitle;
    sf::Text          _lblDepth, _txtDepth;
    sf::Text          _lblDec, _lblInc;
    sf::Text          _lblStart, _lblManual, _lblBack;

    int               _depthValue     = 4;    // default depth
    bool              _manualSelected = false;
    bool              _playerRed      = true;
    std::string       _srvHost        = "127.0.0.1";
    unsigned          _srvPort        = 5555;
};

} // namespace pages
