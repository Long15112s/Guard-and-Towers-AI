#pragma once

#include <SFML/Graphics.hpp>
#include "../ui/ComboBox.hpp"
#include "../ui/Button.hpp"
#include <vector>


namespace pages {

class PvEConfigPage {
public:
    enum class Action { None, Start, Back};
    PvEConfigPage(sf::RenderWindow& win, sf::Font& font);
    Action handleEvent(const sf::Event& ev);
    void update();
    void draw();
    bool playerIsRed() const;
    std::string selectedAIModel() const;
    int selected() const;
    int selectedDepth() const;

private:
    // Animation & Hintergrund
    float _pulseTime = 0.f;
    float _vsTime    = 0.f;
    sf::Color _currentBg;
    std::vector<sf::CircleShape> _particles;

    sf::RenderWindow& _win;
    sf::Font&         _font;
    ui::ComboBox      _combo;
    ui::Button        _btnSwap, _btnStart;
    ui::Button        _btnDepthDec, _btnDepthInc;
    sf::Text          _labelLeft, _labelRight;
    sf::Text          _vs;          // Wogender VS-Text
    sf::Text          _labelDepth, _txtDepth;
    sf::Text          _lblDec, _lblInc;
    sf::Text          _lblSwap, _lblStart;
    ui::Button _btnBack;      // ← Back‑Button
    sf::Text   _lblBack;      // ← Back‑Label
    bool              _playerRed = true;
    int               _depthValue = 1;

    // statischer Hintergrund
    sf::Texture                     _bgTexture;
    sf::Sprite                      _bgSprite;

    // Button-Textur für alle Buttons
    sf::Texture                     _buttonTexture;
};

} // namespace pages
