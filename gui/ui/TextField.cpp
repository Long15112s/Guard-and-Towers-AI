#include "TextField.hpp"
#include <cmath>
#include <algorithm>
using namespace ui;

TextField::TextField(const sf::Font& font, unsigned charSize)
{
    // Box-Default
    _box.setFillColor(sf::Color::White);
    _box.setOutlineColor(sf::Color::Black);
    _box.setOutlineThickness(1);

    // Text
    _text.setFont(font);
    _text.setCharacterSize(charSize);
    _text.setFillColor(sf::Color::Black);

    // Caret
    _caret.setSize({1.f, float(charSize*1.2f)});
    _caret.setFillColor(sf::Color::Black);
}

void TextField::setPosition(sf::Vector2f pos) {
    _box.setPosition(pos);
    _text.setPosition(pos + sf::Vector2f(4,2));
}
void TextField::setSize(sf::Vector2f sz) {
    _box.setSize(sz);
}

void TextField::setString(const std::string& s) {
    _text.setString(s);
    _caretPos = s.size();
    _selStart = _selEnd = _caretPos;
    _history.clear();
}
std::string TextField::getString() const {
    // hier kommt wirklich *nur* der Inhalt des sf::Text
    return _text.getString().toAnsiString();
}


void TextField::saveHistory() {
    _history.push_back(_text.getString());
    if(_history.size()>50) _history.erase(_history.begin());
}

float TextField::caretPixelX(std::size_t pos) const {
    sf::String sub = _text.getString().substring(0, pos);
    sf::Text   tmp(sub, *_text.getFont(), _text.getCharacterSize());
    return _text.getPosition().x + tmp.getLocalBounds().width;
}

void TextField::handleEvent(const sf::Event& ev, const sf::RenderWindow& win) {
    // Maus-Klicks
    if(ev.type==sf::Event::MouseButtonPressed) {
        sf::Vector2f p = win.mapPixelToCoords({ev.mouseButton.x, ev.mouseButton.y});
        if(_box.getGlobalBounds().contains(p)) {
            // Aktivieren + Doppelklick?
            _active = true;
            if(_clickClock.getElapsedTime().asMilliseconds()<400) {
                _clickCount++;
            } else {
                _clickCount = 1;
            }
            _clickClock.restart();
            // Position berechnen
            float best = 1e6f; std::size_t bp=0;
            for(std::size_t i=0;i<=_text.getString().getSize();++i){
                float dx = std::abs(caretPixelX(i)-p.x);
                if(dx<best){ best=dx; bp=i; }
            }
            _caretPos = bp;
            if(_clickCount==2) {
                // Doppel-Klick: alles markieren
                std::size_t n = _text.getString().getSize();
                _selStart = 0;
                _selEnd   = n;
                _caretPos = n;
            } else {
                _selStart=_selEnd=_caretPos;
            }
            _selecting = true;
        } else {
            _active = false;
        }
    }

    // Maus bewegen (Text-Auswahl | Drag&Drop wird extern gehandhabt)
    if(ev.type==sf::Event::MouseMoved && _selecting && _active) {
        sf::Vector2f p = win.mapPixelToCoords({ev.mouseMove.x, ev.mouseMove.y});
        float best = 1e6f; std::size_t bp=_caretPos;
        for(std::size_t i=0;i<=_text.getString().getSize();++i){
            float dx = std::abs(caretPixelX(i)-p.x);
            if(dx<best){ best=dx; bp=i; }
        }
        _caretPos = bp;
        _selEnd   = _caretPos;
    }

    // Maus loslassen: Auswahl beenden
    if(ev.type==sf::Event::MouseButtonReleased && _selecting) {
        _selecting = false;
    }

    // Tastatur nur wenn aktiv
    if(!_active) return;

    // TextEntered (druckbare Zeichen)
    if(ev.type==sf::Event::TextEntered) {
        auto u = ev.text.unicode;
        if(u>=32 && u<128){
            saveHistory();
            // Selektion löschen
            std::size_t a=std::min(_selStart,_selEnd),
                        b=std::max(_selStart,_selEnd);
            std::string s = _text.getString();
            if(a!=b){ s.erase(a,b-a); _caretPos=a; }
            // einfügen
            s.insert(_caretPos,1,char(u));
            _caretPos++;
            _selStart=_selEnd=_caretPos;
            _text.setString(s);
        }
    }

    // Steuerbefehle
    if(ev.type==sf::Event::KeyPressed) {
        bool ctrl  = ev.key.control;
        bool shift = ev.key.shift;
        auto kc    = ev.key.code;

        if(ctrl && kc==sf::Keyboard::Z && !_history.empty()) {
            // Undo
            _text.setString(_history.back());
            _history.pop_back();
            _caretPos = _text.getString().getSize();
            _selStart=_selEnd=_caretPos;
        }
        else if(ctrl && kc==sf::Keyboard::A) {
            // Alles markieren
            std::size_t n = _text.getString().getSize();
            _selStart=0; _selEnd=n; _caretPos=n;
        }
        else if(ctrl && kc==sf::Keyboard::C && _selStart!=_selEnd){
            auto a=std::min(_selStart,_selEnd), b=std::max(_selStart,_selEnd);
            sf::Clipboard::setString(_text.getString().substring(a,b-a));
        }
        else if(ctrl && kc==sf::Keyboard::X && _selStart!=_selEnd){
            saveHistory();
            std::size_t a=std::min(_selStart,_selEnd), b=std::max(_selStart,_selEnd);
            auto s = std::string(_text.getString().substring(a,b-a));
            sf::Clipboard::setString(s);
            std::string st = _text.getString(); st.erase(a,b-a);
            _text.setString(st);
            _caretPos = _selStart = a; _selEnd = a;
        }
        else if(ctrl && kc==sf::Keyboard::V){
            saveHistory();
            auto clip = sf::Clipboard::getString().toAnsiString();
            std::size_t a=std::min(_selStart,_selEnd), b=std::max(_selStart,_selEnd);
            std::string st = _text.getString();
            if(a!=b){ st.erase(a,b-a); _caretPos=a; }
            st.insert(_caretPos,clip);
            _caretPos += clip.size();
            _selStart=_selEnd=_caretPos;
            _text.setString(st);
        }
        else if(kc==sf::Keyboard::Backspace){
            saveHistory();
            std::size_t a=std::min(_selStart,_selEnd), b=std::max(_selStart,_selEnd);
            std::string st = _text.getString();
            if(a!=b){ st.erase(a,b-a); _caretPos=a; }
            else if(_caretPos>0){ st.erase(_caretPos-1,1); _caretPos--; }
            _selStart=_selEnd=_caretPos;
            _text.setString(st);
        }
        else if(kc==sf::Keyboard::Delete){
            saveHistory();
            std::size_t a=std::min(_selStart,_selEnd), b=std::max(_selStart,_selEnd);
            std::string st = _text.getString();
            if(a!=b){ st.erase(a,b-a); _caretPos=a; }
            else if(_caretPos<st.size()){ st.erase(_caretPos,1); }
            _selStart=_selEnd=_caretPos;
            _text.setString(st);
        }
        else if(kc==sf::Keyboard::Left || kc==sf::Keyboard::Right){
            int d = (kc==sf::Keyboard::Right?+1:-1);
            std::size_t np = std::clamp<int>(int(_caretPos)+d, 0, _text.getString().getSize());
            if(shift){
                _caretPos=np; _selEnd=np;
            } else {
                _caretPos=np; _selStart=_selEnd=np;
            }
        }

        // Aufräum-Timer für Blink
        _showCaret = true;
        _clickClock.restart();
    }
}

void TextField::update() {
    if(_active && _clickClock.getElapsedTime().asSeconds()>0.5f){
        _showCaret = !_showCaret;
        _clickClock.restart();
    }
}

void TextField::draw(sf::RenderTarget& rt) const {
    // Box
    rt.draw(_box);

    // Selektion
    if(_selStart!=_selEnd){
        float x1 = caretPixelX(std::min(_selStart,_selEnd)),
              x2 = caretPixelX(std::max(_selStart,_selEnd));
        sf::RectangleShape sel({x2-x1, _box.getSize().y-4});
        sel.setPosition(x1, _box.getPosition().y+2);
        sel.setFillColor({100,150,255,120});
        rt.draw(sel);
    }

    // Text
    rt.draw(_text);

    // Caret
    if(_active && _showCaret){
        float x = caretPixelX(_caretPos);
        float y = _box.getPosition().y + 2;
        // lokale Kopie verwenden, weil draw() const ist
        sf::RectangleShape caret = _caret;
        caret.setPosition(x, y);
        rt.draw(caret);
    }
}
