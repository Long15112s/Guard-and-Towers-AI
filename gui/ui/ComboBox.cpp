// gui/ui/ComboBox.cpp
#include "ComboBox.hpp"

namespace ui {

ComboBox::ComboBox(const sf::Vector2f& size)
{
    // Font laden oder per Setter von außen reinreichen
    _font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    _box.setSize(size);
    _box.setFillColor(sf::Color::White);
    _box.setOutlineColor(sf::Color::Black);
    _box.setOutlineThickness(1.f);

    _selectedText.setFont(_font);
    _selectedText.setCharacterSize(18);
    _selectedText.setFillColor(sf::Color::Black);
}

void ComboBox::setPosition(const sf::Vector2f& pos) {
    _box.setPosition(pos);
    _selectedText.setPosition(pos.x + 5.f, pos.y + ( _box.getSize().y - _selectedText.getCharacterSize() )/2 );
    // Drop-Items werden in handleEvent geupdated
}

void ComboBox::addItem(const std::string& label) {
    _items.push_back(label);
    sf::Text t(label, _font, 18);
    t.setFillColor(sf::Color::Black);
    _dropItems.push_back(t);
}

void ComboBox::select(std::size_t idx) {
    if (idx < _items.size()) {
        _current = idx;
        _selectedText.setString(_items[idx]);
    }
}

std::size_t ComboBox::selected() const {
    return _current;
}

void ComboBox::handleEvent(const sf::Event& ev, sf::RenderWindow& win) {
    auto mouse = sf::Mouse::getPosition(win);
    if (ev.type == sf::Event::MouseButtonReleased &&
        _box.getGlobalBounds().contains(static_cast<sf::Vector2f>(mouse))) {
        _open = !_open;
    }
    if (_open) {
        // einfache Hit-Test-Logik für Drop-Items
        for (std::size_t i = 0; i < _dropItems.size(); ++i) {
            sf::FloatRect r(_box.getPosition().x,
                            _box.getPosition().y + _box.getSize().y * (i+1),
                            _box.getSize().x,
                            _box.getSize().y);
            if (ev.type == sf::Event::MouseButtonReleased &&
                r.contains(static_cast<sf::Vector2f>(mouse))) {
                select(i);
                _open = false;
            }
        }
    }
}

void ComboBox::draw(sf::RenderTarget& rt, sf::RenderStates states) const {
    rt.draw(_box, states);
    rt.draw(_selectedText, states);
    if (_open) {
        for (std::size_t i = 0; i < _dropItems.size(); ++i) {
            sf::RectangleShape rect(_box);
            rect.setPosition(_box.getPosition().x,
                             _box.getPosition().y + _box.getSize().y * (i+1));
            rect.setFillColor({240,240,240});
            rt.draw(rect, states);

            // erst kopieren, dann verschieben und zeichnen
            sf::Text txt = _dropItems[i];
            txt.setPosition(
              rect.getPosition().x + 5.f,
              rect.getPosition().y + (rect.getSize().y - txt.getCharacterSize())/2
            );
            rt.draw(txt, states);
        }
    }
}

} // namespace ui
