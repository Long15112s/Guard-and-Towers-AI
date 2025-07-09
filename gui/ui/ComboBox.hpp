// gui/ui/ComboBox.hpp
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

namespace ui {

class ComboBox : public sf::Drawable {
public:
    // Größe des sichtbaren Feldes (ohne „aufgeklappte“ Liste)
    ComboBox(const sf::Vector2f& size);

    void setPosition(const sf::Vector2f& pos);
    void addItem(const std::string& label);
    void select(std::size_t index);
    std::size_t selected() const;

    // muss das RenderWindow übergeben, damit das Event richtig gehandelt wird
    void handleEvent(const sf::Event& ev, sf::RenderWindow& win);

    // Draw-Aufruf: malt Box + aktuell ausgewählten Eintrag (+ Liste, wenn offen)
    virtual void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;

private:
    sf::RectangleShape         _box;
    sf::Text                   _selectedText;
    sf::Font                   _font;         // ihr könnt den globalen Font referenzieren oder laden
    std::vector<std::string>   _items;
    std::size_t                _current = 0;
    bool                       _open    = false;
    std::vector<sf::Text>      _dropItems;   // Texte für die ausgeklappte Liste
};

} // namespace ui
