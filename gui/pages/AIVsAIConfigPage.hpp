#pragma once

#include <SFML/Graphics.hpp>
#include "../ui/ComboBox.hpp"
#include "../ui/Button.hpp"
#include <vector>
#include "AIRegistry.hpp"

namespace pages {

class AIVsAIConfigPage {
public:
    enum class Action { None, Start };

    AIVsAIConfigPage(sf::RenderWindow& win, const sf::Font& font);

    /// verarbeitet Klicks, verändert internen State, gibt Start zurück, wenn Los gedrückt
    Action handleEvent(const sf::Event& ev);

    /// für Animationen: Gradient, Partikel, VS-Wobble
    void update();

    /// zeichnet die gesamte Seite (ohne clear/display)
    void draw();

    // Getter für ausgewählte Modelle und Tiefen
    int         selectedRed()       const { return _comboRed.selected(); }
    std::string redAIModel() const { return ::AIRegistry()[_comboRed.selected()].name; }
    int         selectedRedDepth()  const { return _depthValueRed; }

    int         selectedBlue()      const { return _comboBlue.selected(); }
    std::string blueAIModel() const { return ::AIRegistry()[_comboBlue.selected()].name; }
    int         selectedBlueDepth() const { return _depthValueBlue; }

private:
    sf::RenderWindow& _win;
    const sf::Font&   _font;

    // Animation & Hintergrund
    float _pulseTime = 0.f;
    float _vsTime    = 0.f;
    sf::Color _currentBg;
    std::vector<sf::CircleShape> _particles;

    // Controls
    ui::ComboBox _comboRed, _comboBlue;
    ui::Button   _btnDecRed, _btnIncRed, _btnDecBlue, _btnIncBlue, _btnStart;
    sf::Text     _vsLabel;
    sf::Text     _labelRed, _labelDepthRed, _txtDepthRed, _lblDecRed, _lblIncRed;
    sf::Text     _labelBlue, _labelDepthBlue, _txtDepthBlue, _lblDecBlue, _lblIncBlue;
    sf::Text     _lblStart;
    
    // statisches Fullscreen-Background
    sf::Texture    _bgTexture;
    sf::Sprite     _bgSprite;

    // Button-Textur für alle Widgets
    sf::Texture    _buttonTexture;

    int _depthValueRed   = 1;
    int _depthValueBlue  = 1;
};

} // namespace pages