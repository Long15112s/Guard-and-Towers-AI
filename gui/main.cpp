#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "pages/StartPage.hpp"
#include "pages/PvEConfigPage.hpp"
#include "pages/AIVsAIConfigPage.hpp"
#include "pages/NetworkConfigPage.hpp"
#include "pages/GamePage.hpp"

#include "ai/AIRegistry.hpp"

#include <memory>
#include <iostream>
#include <typeinfo>


int main()
{   

    bool inNetConfig = false;

    // --- Fenster und Layout ---
    const float TILE = 80.f;
    const int   M_L  = 60, M_T = 70, M_B = 140;
    const unsigned PANEL_WIDTH = 400;     // statt 330
    const unsigned W = M_L + int(7 * TILE) + PANEL_WIDTH;
    const unsigned H = M_T + int(7 * TILE) + M_B;

    sf::RenderWindow window(
        sf::VideoMode(W, H),
        "Guard & Towers",
        sf::Style::Titlebar | sf::Style::Close
    );
    window.setFramerateLimit(60);
    window.setView(sf::View(sf::FloatRect(0, 0, float(W), float(H))));

    // --- Font laden ---
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"))
        return 1;
    
    // --- Hintergrund-Musik laden & starten ---
    sf::Music bgm;
    if (!bgm.openFromFile("assets/music.wav")) {
        std::cerr << "Fehler: Konnte music.mp3 nicht laden\n";
    } else {
        bgm.setLoop(true);       // Dauerschleife
        bgm.setVolume(1.f);     // Lautstärke 0–100 (hier leise)
        bgm.play();
    }

    // --- Modus-Enum und Anfangszustand ---
    enum class Mode { Start, ConfigPvE, ConfigAIvAI, PvP, PvE, AIvAI, ConfigNetwork, Network };
    Mode mode = Mode::Start;

    // --- Seiten instanziieren ---
    pages::StartPage      startPage(window, font, M_T);
    pages::PvEConfigPage     pvEpage(window, font);
    pages::AIVsAIConfigPage  aivsaipage(window, font);
    pages::NetworkConfigPage netCfg(window, font);
    pages::GamePage       gamePage(window, TILE, M_L, M_T, M_B);


    // --- Hauptschleife ---
    while (window.isOpen())
    {
        bool consumed = false;
        sf::Event ev;
        while (window.pollEvent(ev))
        {
            if (ev.type == sf::Event::Closed)
            {
                window.close();
                break;
            }

            // --- START PAGE ---
            if (mode == Mode::Start)
            {
                auto action = startPage.handleEvent(ev);

                if (action == pages::StartPage::Action::PvP)
                {
                    std::cout << "[Main] Switching to PvP mode\n";
                    gamePage.setMode(pages::Mode::HumanVsHuman);
                    gamePage.reset();
                    mode = Mode::PvP;
                    consumed = true;
                }
                else if (action == pages::StartPage::Action::PvE)
                {
                    // Anstatt direkt PvE: gehe zur KI-Auswahl
                    std::cout << "[Main] Switching to SelectKI\n";
                    mode = Mode::ConfigPvE;
                    consumed = true;
                }
                else if (action == pages::StartPage::Action::AIvAI)
                {
                    std::cout << "[Main] Switching to AIvsAI mode\n";
                    gamePage.setMode(pages::Mode::AIvsAI);
                    gamePage.reset();
                    mode = Mode::ConfigAIvAI;
                    consumed = true;
                }
                else if (action == pages::StartPage::Action::Online) {
                    mode = Mode::ConfigNetwork;
                    consumed = true;
                }
                else if (action == pages::StartPage::Action::Exit)
                {
                    std::cout << "[Main] Exit clicked\n";
                    window.close();
                    break;
                }
            }
            // --- MODEL SELECT PAGE ---
                        // --- PvE-Konfig ---
            else if (mode == Mode::ConfigPvE) {
                auto a = pvEpage.handleEvent(ev);
                if (a == pages::PvEConfigPage::Action::Start) {
                    bool redPlays = pvEpage.playerIsRed();
                    gamePage.setMode(pages::Mode::HumanVsAI);
                    gamePage.setHumanIsRed(redPlays);

                    // ← Hier kommt dein Factory-Code hin:
                    int idx   = pvEpage.selected();        // Combo-Index 0..N-1
                    int depth = pvEpage.selectedDepth();   // musst du in PvEConfigPage noch exposen
                    auto ai   = AIRegistry()[idx].create(depth);
                    if (pvEpage.playerIsRed())
                        gamePage.setBlueAI(std::move(ai));
                    else
                        gamePage.setRedAI (std::move(ai));

                    gamePage.reset();
                    mode = Mode::PvE;
                    consumed = true;
                }
            }
            else if (mode == Mode::ConfigNetwork) {
                auto act = netCfg.handleEvent(ev);

                if (act == pages::NetworkConfigPage::Action::StartAI) {
                    // AI-Modus
                    gamePage.setMode(pages::Mode::Network);
                    gamePage.reset();
                    gamePage.setNetworkConfig(netCfg.serverHost(),
                                            netCfg.serverPort(),
                                            netCfg.selectedDepth());
                    gamePage.setNetworkManual(false);

                    // ausgewählte AI bauen
                    auto it = std::find_if(
                        AIRegistry().begin(), AIRegistry().end(),
                        [&](auto const& e){ return e.name == netCfg.selectedAIModel(); }
                    );
                    int idx = int(it - AIRegistry().begin());
                    auto aiModel = AIRegistry()[idx].create(netCfg.selectedDepth());
                    gamePage.setNetworkAI(std::move(aiModel));

                    mode = Mode::Network;
                    consumed = true;
                }
                else if (act == pages::NetworkConfigPage::Action::StartManual) {
                    // Manueller Modus (eigene Züge per Drag&Drop)
                    gamePage.setMode(pages::Mode::Network);
                    gamePage.reset();
                    gamePage.setNetworkConfig(netCfg.serverHost(),
                                            netCfg.serverPort(),
                                            netCfg.selectedDepth());
                    gamePage.setNetworkManual(true);
                    mode = Mode::Network;
                    consumed = true;
                }
                else if (act == pages::NetworkConfigPage::Action::Back) {
                    mode = Mode::Start;
                    consumed = true;
                }
            }



            // --- AI-vs-AI-Konfig ---
            else if (mode == Mode::ConfigAIvAI) {
                auto a = aivsaipage.handleEvent(ev);
                if (a == pages::AIVsAIConfigPage::Action::Start) {
                    gamePage.setMode(pages::Mode::AIvsAI);

                    // Factory für Rot
                    int idxR    = aivsaipage.selectedRed();      // Combo-Index für Rot
                    int depthR  = aivsaipage.selectedRedDepth(); // Tiefe für Rot
                    auto aiRed  = AIRegistry()[idxR].create(depthR);
                    gamePage.setRedAI(std::move(aiRed));

                    // Factory für Blau
                    int idxB    = aivsaipage.selectedBlue();     
                    int depthB  = aivsaipage.selectedBlueDepth();
                    auto aiBlue = AIRegistry()[idxB].create(depthB);
                    gamePage.setBlueAI(std::move(aiBlue));

                    gamePage.reset();
                    mode = Mode::AIvAI;
                    consumed = true;
                }
            }
            // --- GAME PAGE (PvP, PvE, AIvAI) ---
            else {
                gamePage.handleEvent(ev);
                if (!gamePage.isActive()) {
                    std::cout << "[Main] Back to StartPage\n";
                    mode = Mode::Start;
                    consumed = true;
                }
            }

            if (consumed)
                break;
        }

        // --- Rendern ---
        window.clear({200,200,200});

        if (mode == Mode::Start) {
            startPage.draw();
        }
        else if (mode == Mode::ConfigPvE) {
            pvEpage.draw();
        }
        else if (mode == Mode::ConfigAIvAI) {
            aivsaipage.draw();
        }
        else if (mode == Mode::ConfigNetwork) {
            netCfg.draw();
        }
        else {
            gamePage.update();
            gamePage.draw();
        }

        window.display();
    }

    return 0;
}
