#pragma once
#include <SFML/Graphics.hpp>
#include <string>

namespace ui {

/// A simple clickable rectangle + label
struct Button : public sf::Drawable {

  sf::RectangleShape box;
  sf::Text           label;

  Button() = default;
  /// construct with an initial size
  Button(sf::Vector2f size) { box.setSize(size); }

  /// position the whole widget
  void setPosition(sf::Vector2f p)        { box.setPosition(p); }
  /// hit‑box for clicks, etc.
  sf::FloatRect getGlobalBounds() const   { return box.getGlobalBounds(); }
  /// for centering your label
  sf::Vector2f  getSize() const           { return box.getSize(); }

  bool contains(sf::Vector2f p) const {
    return box.getGlobalBounds().contains(p);
  }
  
    virtual void draw(sf::RenderTarget& rt, sf::RenderStates) const override {
    rt.draw(box);
    rt.draw(label);
  }
};

/// factory if you still want one
Button makeButton(const sf::Font& font,
                  const std::string& txt,
                  sf::Vector2f pos,
                  sf::Vector2f size);

} // namespace ui
