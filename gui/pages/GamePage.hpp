// GamePage.hpp
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include <vector>
#include <string>

#include "../src/MoveGenerator.hpp"   // Move, MoveGenerator
#include "../src/Board.hpp"           // Board
#include "../../src/ai/AI.hpp"              // ai::AI
#include "../ui/Button.hpp"
#include "../ui/TextField.hpp"
#include "NetworkConnector.hpp"

namespace pages {

enum class Mode { HumanVsHuman, HumanVsAI, AIvsAI, Network };

struct Entry {
    Move        move;
    std::string fen;
    Color       mover;
    Entry() = default;
    Entry(const Move& m, const std::string& f, Color c)
      : move(m), fen(f), mover(c) {}
};

struct GuiStyle {
    static const sf::Color BG;
    static const sf::Color PANEL;
    static const sf::Color ENTRY_EVEN;
    static const sf::Color ENTRY_ODD;
    static const sf::Color TEXT;
    static const sf::Color BTN_NORMAL;
    static const sf::Color BTN_HOVER;
    static constexpr float PANEL_PADDING    = 10.f;
    static constexpr float TOP_BAR_H        = 30.f;
    static constexpr float HISTORY_LINE_H   = 20.f;
    static constexpr float HISTORY_PADDING  = 4.f;
};

class GamePage {


public:
   GamePage(sf::RenderWindow& window,
            float tileSize, int marginL, int marginT, int marginB);

    void draw();
    void handleEvent(const sf::Event& ev);
    void update();
    void reset();
    void setMode(Mode m);
    void setRedAI(std::unique_ptr<ai::AI> a);
    void setBlueAI(std::unique_ptr<ai::AI> a);
    void swapSides();
    void setHumanIsRed(bool r) { _humanIsRed = r; }
    bool isActive() const     { return _active; }


    void setNetworkConfig(const std::string& host, unsigned port, int aiDepth);
    void setNetworkAI(std::unique_ptr<ai::AI> ai);
    void setNetworkManual(bool m) { _networkManual = m; }
    void resetNetwork();
    bool playerIsRed() const { return _playerId; }



private:
    // Netzwerk-Konfigurationsparameter
  // für Network-Mode
    std::string                     _srvHost;
    unsigned                        _srvPort;
    int                             _networkAIDepth = 0;
    std::unique_ptr<NetworkConnector> _net;
    bool   _networkManual = false;
    std::unique_ptr<ai::AI>           _networkAI;
    int                                _playerId = -1;
    void applyMoveLocally(const Move& mv, ::Color mover);

    // Utility
    int           pixelToSq(sf::Vector2i pix) const;
    sf::Vector2f  sqToPos(int sq) const;
    void          computeLegal(int from);
    bool          isLegal(int to) const;
    void          runAIMove();

    private:
    sf::Texture       _boardTexture;
    sf::RectangleShape _boardBackground;
    sf::Texture       _blueTexture;
    sf::Texture       _redTexture;
    sf::Texture       _buttonTexture;

    // In GamePage.hpp, im private-Bereich ergänzen:
    sf::Texture _bgTexture;
    sf::Sprite  _bgSprite;
    sf::Text    _lblTitle;


    sf::Font _font;

    sf::Texture   _frameTex;
    sf::Sprite    _frameSpr;
    sf::RectangleShape _frameShape;

        // full‐screen background
    sf::Texture _fullBgTex;
    sf::Sprite  _fullBgSpr;

    // — Style & geometry —
    sf::RenderWindow&        _win;
    float                     _TILE;
    int                       _M_L, _M_T, _M_B;

    // — Game state —
    Board                     _board;
    Color                     _side;
    Mode                      _mode = Mode::HumanVsHuman;
    bool                      _gameOver = false;
    bool                      _humanIsRed = true;
    bool                      _active = true;
    std::string               _winMessage;
    std::string               _startFen;

    // — UI controls —
    ui::TextField             _fenField;
    ui::Button                _loadBtn, _newGameBtn, _backBtn;
    sf::Text                  _loadLabel, _newGameLabel, _backLabel;
    sf::Text                  _modeLabel;

    ui::Button                _btnSwap;
    sf::Text                  _lblSwap;

    // — Drag & drop —
    bool                      _dragging = false;
    bool                      _dragIsGuard = false;
    int                       _fromSq = -1;
    int                       _dragMovedCount = 0;
    int                       _dragHoverTo = -1;
    sf::RectangleShape        _dragRect;
    sf::CircleShape           _dragShape;
    sf::Text                  _dragText;
    std::vector<Move>         _legalMoves;

    // — History —
    std::vector<Entry>        _history;
    float                     _historyScrollOffset = 0.f;
    int                       _selectedHistoryIndex = 0;

    // — AIs —
    std::unique_ptr<ai::AI>   _redAI, _blueAI;

    // — Goal squares —
    static constexpr int      RED_GOAL_SQ = 3;
    static constexpr int      BLUE_GOAL_SQ = 45;

    // --- Sound für Züge ---
    sf::SoundBuffer _moveBuffer;
    sf::Sound       _moveSound;
};

// --- GuiStyle definitions ---
inline const sf::Color GuiStyle::BG          (30,  30,  40);
inline const sf::Color GuiStyle::PANEL       (50,  50,  70);
inline const sf::Color GuiStyle::ENTRY_EVEN  (255, 255, 255); // Weiß
inline const sf::Color GuiStyle::ENTRY_ODD   (230, 230, 230); // Hellgrau
inline const sf::Color GuiStyle::TEXT        (240, 240, 240);
inline const sf::Color GuiStyle::BTN_NORMAL  (100, 100, 140);
inline const sf::Color GuiStyle::BTN_HOVER   (140, 140, 180);

} // namespace pages
