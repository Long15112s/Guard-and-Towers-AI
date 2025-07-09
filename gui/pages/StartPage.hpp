#pragma once

#include <SFML/Graphics.hpp>
#include "../ui/Button.hpp"
#include <vector>

namespace pages {

class StartPage {
public:
    enum class Action { None, PvP, PvE, AIvAI, Online, Exit };

    StartPage(sf::RenderWindow& win, const sf::Font& font, float top);
    Action handleEvent(const sf::Event& ev);
    void draw();

private:
    sf::RenderWindow& _win;
    const sf::Font&   _font;
    float             _top;

    // Puls-Animation
    float             _pulseTime = 0.f;
    // Titel-Wellen-Animation
    float             _titleTime = 0.f;
    // aktueller Hintergrund-Farbwert
    sf::Color         _currentBg;
    // Partikel für Hintergrund
    std::vector<sf::CircleShape> _particles;

    // UI-Buttons
    ui::Button _btnPvP;
    ui::Button _btnPvE;
    ui::Button _btnAIvAI;
    ui::Button _btnExit;
    ui::Button _btnOnline;
    int        _hover = -1;

    // statischer Hintergrund
    sf::Texture                     _bgTexture;
    sf::Sprite                      _bgSprite;

    // Button–Textur
    sf::Texture                     _buttonTexture;
    sf::Text                       _title, _titleShadow;
    sf::Font   _titleFont;

    Action     _action = Action::None;
};

} // namespace pages
