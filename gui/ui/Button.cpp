// gui/ui/Button.cpp
#include "Button.hpp"

namespace ui {

Button makeButton(const sf::Font& font,
                  const std::string& txt,
                  sf::Vector2f pos,
                  sf::Vector2f size)
{
    Button b;
    b.box.setPosition(pos);
    b.box.setSize(size - sf::Vector2f(4,4));
    b.box.setFillColor({180,180,180});
    b.box.setOutlineColor(sf::Color::Black);
    b.box.setOutlineThickness(2);

    b.label.setFont(font);
    b.label.setString(txt);
    b.label.setCharacterSize(20);
    auto lb = b.label.getLocalBounds();
    b.label.setPosition(
      pos.x + (size.x - lb.width )/2  - lb.left,
      pos.y + (size.y - lb.height)/2  - lb.top
    );

    return b;
}

} // namespace ui
