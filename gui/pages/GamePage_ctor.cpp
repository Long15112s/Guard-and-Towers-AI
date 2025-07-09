#include "GamePage.hpp"
#include <SFML/Audio.hpp> 
#include "../../src/FEN.hpp"
#include <iostream>
#include <algorithm>
#include <cstring>

int pages::GamePage::pixelToSq(sf::Vector2i pix) const {
    int x = pix.x - _M_L, y = pix.y - _M_T;
    int f = x / int(_TILE), r = 6 - (y / int(_TILE));
    if (f<0||f>6||r<0||r>6) return -1;
    return r*7 + f;
}
sf::Vector2f pages::GamePage::sqToPos(int sq) const {
    int r = sq/7, f = sq%7;
    return { _M_L+f*_TILE+_TILE/2.f, _M_T+(6-r)*_TILE+_TILE/2.f };
}
void pages::GamePage::computeLegal(int from) {
    _legalMoves.clear();
    std::vector<Move> all;
    MoveGenerator::generate(_board, all);
    for(auto&m:all) if(m.from==from) _legalMoves.push_back(m);
}
bool pages::GamePage::isLegal(int to) const {
    for(auto&m:_legalMoves) if(m.to==to) return true;
    return false;
}

void pages::GamePage::setMode(Mode m) {
    _mode = m;
    static const char* names[] = {
        "Player vs Player",
        "Player vs AI",
        "AI vs AI"
    };
    _modeLabel.setString(names[int(m)]);
    // ← new: always re‑anchor label after any change
    float x = _M_L + 7*_TILE + GuiStyle::PANEL_PADDING + GuiStyle::HISTORY_PADDING + 20.f;
    float y = _M_T + GuiStyle::HISTORY_PADDING;
    _modeLabel.setPosition(x, y);
}

using namespace sf;
using namespace std;

namespace pages {

// horizontaler Versatz für Panel/Buttons/Labels
static constexpr float PANEL_OFFSET_X = 50.f;

pages::GamePage::GamePage(sf::RenderWindow& window,
                          float tile, int mL, int mT, int mB)
: _win(window)
, _TILE(tile), _M_L(mL), _M_T(mT), _M_B(mB)
, _fenField(/*font=*/_font, /*size=*/18)
, _active(true)
{
    // --- Full-Screen-Hintergrund laden ---
    if (!_fullBgTex.loadFromFile("assets/bg4.png")) {
        std::cerr << "Fehler: Konnte bg4.png nicht laden\n";
    } else {
        _fullBgSpr.setTexture(_fullBgTex);
        // Fenster- und Textur-Größe holen
        auto ws = _win.getSize();
        auto ts = _fullBgTex.getSize();
        // auf Fenster skalieren
        _fullBgSpr.setScale(
        float(ws.x) / float(ts.x),
        float(ws.y) / float(ts.y)
        );

        // ↓ hier nur das Background-Sprite nach rechts verschieben:
        constexpr float BG_OFFSET_X = 100.f;  // z.B. 100px nach rechts
        _fullBgSpr.setPosition(BG_OFFSET_X, 0.f);
    }

    // --- Board-Hintergrund laden ---
    if (!_boardTexture.loadFromFile("assets/wood2.png")) {
        std::cerr << "Fehler: Konnte wood.jpeg nicht laden\n";
    } else {
        // Größe so skalieren, dass sie exakt 7*TILE × 7*TILE abdeckt
        _boardBackground.setTexture(&_boardTexture);
        _boardBackground.setSize({7*_TILE, 7*_TILE});
        _boardBackground.setPosition(_M_L, _M_T);
    }
    
   // --- Steine-Texturen laden ---
   if (!_blueTexture.loadFromFile("assets/blue.png")) {
       std::cerr << "Fehler: Konnte blue.png nicht laden\n";
   }
   if (!_redTexture.loadFromFile("assets/red.png")) {
       std::cerr << "Fehler: Konnte rot.png nicht laden\n";
   }

    if (!_frameTex.loadFromFile("assets/frame.png")) {
        std::cerr << "Fehler: Konnte frame.png nicht laden\n";
    } else {
        _frameSpr.setTexture(_frameTex);
                // Skalierung ermitteln und anwenden
        float boardW = 7 * _TILE + 70.f;
        float boardH = 7 * _TILE + 70.f;
        // Einmalig Größe abrufen und in frameSize speichern
        auto frameSize = _frameTex.getSize();

        // RectangleShape für den Rahmen
        _frameShape.setSize({ float(frameSize.x), float(frameSize.y) });
        _frameShape.setPosition(_M_L - 35.f, _M_T - 35.f);
        _frameShape.setTexture(&_frameTex);
        _frameShape.setScale(boardW / frameSize.x,
                           boardH / frameSize.y);
        _frameShape.setPosition(_M_L - 35.f, _M_T - 35.f);

        _frameSpr.setScale(boardW / frameSize.x,
                           boardH / frameSize.y);
        _frameSpr.setPosition(_M_L - 35.f, _M_T - 35.f);
    }


    // --- Mittelalterliche Font laden ---
    if (!_font.loadFromFile("assets/MedievalSharp-Regular.ttf")) {
        std::cerr << "Fehler: Konnte MedievalSharp-Regular.ttf nicht laden\n";
    }

    // --- Button-Textur laden ---
    if (!_buttonTexture.loadFromFile("assets/button.png")) {
        std::cerr << "Fehler: Konnte button.png nicht laden\n";
    }

    // --- Move-Sound laden ---
    if (!_moveBuffer.loadFromFile("assets/move.wav")) {
        std::cerr << "Fehler: Konnte move.wav nicht laden\n";
    } else {
        _moveSound.setBuffer(_moveBuffer);
        _moveSound.setVolume(50.f);  // anpassen: 0–100
    }


    if (_bgTexture.loadFromFile("assets/config.png")) {
      _bgSprite.setTexture(_bgTexture);
      // falls Du das Sprite wirklich in der GamePage zeichnen willst:
      _bgSprite.setScale(
        float(_win.getSize().x)/_bgTexture.getSize().x,
        float(_win.getSize().y)/_bgTexture.getSize().y
      );
    }
    // Titel initialisieren
    _lblTitle.setFont(_font);
    _lblTitle.setString("Guard & Towers");
    _lblTitle.setCharacterSize(30);
    _lblTitle.setFillColor(sf::Color::White);
    auto tb = _lblTitle.getLocalBounds();
    _lblTitle.setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);
    _lblTitle.setPosition(_win.getSize().x/2.f, 50.f);








    // FEN‑Eingabe initialisieren
    const string initial = "r1r11RG1r1r1/2r11r12/3r13/7/3b13/2b11b12/b1b11BG1b1b1 r";
    _board.loadFEN(initial);
    _side = _board.sideToMove();
    _startFen = _board.toFEN();
    _fenField.setPosition({float(_M_L)-3, float(_M_T + 7*_TILE + 50)});
    _fenField.setSize({ _TILE*7.1f, 30 });
    _fenField.setString(_startFen);

    // Button‑Style
    auto styleButton = [&](ui::Button& b, sf::Text& lbl, const string& str){
        b.box.setTexture(&_buttonTexture);
        lbl.setFont(_font);
        lbl.setString(str);
        lbl.setCharacterSize(18);
        lbl.setFillColor(GuiStyle::TEXT);
    };

    // Load-Button
    _loadBtn = ui::Button({80,30});
    styleButton(_loadBtn, _loadLabel, "Load");
    _loadBtn.setPosition({ float(_M_L + PANEL_OFFSET_X - 53.f), float(_M_T + 7*_TILE + 90) });
    {
        auto lb = _loadLabel.getLocalBounds();
        auto btnB = _loadBtn.getGlobalBounds();
        _loadLabel.setPosition(
            btnB.left + (btnB.width  - lb.width )/2 - lb.left,
            btnB.top  + (btnB.height - lb.height)/2 - lb.top
        );
    }

    // NewGame / Back Buttons
    float histX  = _M_L + 7*_TILE + GuiStyle::PANEL_PADDING + PANEL_OFFSET_X;
    float panelH = 7*_TILE + _M_B - _M_T - GuiStyle::TOP_BAR_H - 50.f;
    float btnY   = _M_T + GuiStyle::TOP_BAR_H + panelH -20.f;


    _newGameBtn = ui::Button({120,40});
    styleButton(_newGameBtn, _newGameLabel, "Neues Spiel");
    _newGameBtn.setPosition({ histX + 30.f,          btnY + 50.f });
    {
        auto ngB  = _newGameBtn.getGlobalBounds();
        auto lblN = _newGameLabel.getLocalBounds();
        _newGameLabel.setPosition(
            ngB.left + (ngB.width  - lblN.width )/2 - lblN.left,
            ngB.top  + (ngB.height - lblN.height)/2 - lblN.top
        );
    }

    _backBtn = ui::Button({120,40});
    styleButton(_backBtn, _backLabel, "Back");
    _backBtn.setPosition({ histX + 30.f + 150.f,    btnY + 50.f});
    {
        auto bb   = _backBtn.getGlobalBounds();
        auto lblB = _backLabel.getLocalBounds();
        _backLabel.setFillColor(GuiStyle::TEXT);
        _backLabel.setPosition(
            bb.left + (bb.width  - lblB.width )/2 - lblB.left,
            bb.top  + (bb.height - lblB.height)/2 - lblB.top
        );
    }




    // Drag‑Text
    _dragText.setFont(_font);
    _dragText.setCharacterSize(18);
    _dragText.setFillColor(GuiStyle::TEXT);
    _dragText.setOutlineColor(sf::Color::Black);
    _dragText.setOutlineThickness(2.f);

    // Mode‑Label
    _modeLabel.setFont(_font);
    _modeLabel.setCharacterSize(28);
    _modeLabel.setFillColor(GuiStyle::TEXT);
    setMode(Mode::HumanVsHuman);

    // History scroll offsets
    _historyScrollOffset = 0.f;
    _selectedHistoryIndex = 0;
}

void GamePage::reset() {
    // Board zurücksetzen auf Start‑FEN
    _board.loadFEN(_startFen);
    _fenField.setString(_startFen);
    _side = _board.sideToMove();

    // History initialisieren
    _history.clear();
    _history.emplace_back(Move{}, _startFen, _side);
    _selectedHistoryIndex = 0;
    _historyScrollOffset = 0.f;

    // KI-Zug falls notwendig
    if (_mode == Mode::HumanVsAI && !_humanIsRed) {
        runAIMove();
    }

    // Status zurücksetzen
    _gameOver = false;
    _winMessage.clear();
    _dragging = false;
    _legalMoves.clear();
    _active = true;
}

void GamePage::setRedAI(std::unique_ptr<ai::AI> a) {
    _redAI = std::move(a);
}

void GamePage::setBlueAI(std::unique_ptr<ai::AI> a) {
    _blueAI = std::move(a);
}

void GamePage::swapSides() {
    std::swap(_redAI, _blueAI);
    _side = static_cast<::Color>(1 - int(_side));
    string fen = _board.toFEN();
    fen.back() = (_side == ::Color::Red ? 'r' : 'b');
    _board.loadFEN(fen);
    _fenField.setString(fen);
}



} // namespace pages
