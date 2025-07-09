// gui/ui/TextField.hpp
#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window/Clipboard.hpp>
#include <vector>

namespace ui {

/// Einfache Text-Box mit Caret, Selektion, Undo, Copy/Paste, Doppelklick‐Markierung
class TextField {
public:
    TextField(const sf::Font& font, unsigned charSize);
    void setPosition(sf::Vector2f pos);
    void setSize(sf::Vector2f size);
    void setString(const std::string& s);
    std::string getString() const;
    void handleEvent(const sf::Event& ev, const sf::RenderWindow& win);
    void update();
    void draw(sf::RenderTarget& rt) const;

private:
    sf::RectangleShape _box;
    sf::Text           _text;
    sf::RectangleShape _caret;
    bool               _active    = false;
    std::size_t        _caretPos  = 0, _selStart = 0, _selEnd = 0;
    bool               _selecting = false, _showCaret = true;
    std::vector<std::string> _history;
    sf::Clock          _clickClock;
    int                _clickCount = 0;
    float caretPixelX(std::size_t pos) const;
    void  saveHistory();
};

} // namespace ui
