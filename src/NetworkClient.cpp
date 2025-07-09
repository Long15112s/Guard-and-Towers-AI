// src/NetworkClient.cpp

#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <nlohmann/json.hpp>

#include "Board.hpp"
#include "MoveGenerator.hpp"
#include "ai/MinimaxAI.hpp"

using json = nlohmann::json;
using namespace std::chrono_literals;
using namespace std;
using namespace ai;

// Hilfsfunktion: extrahiere das erste vollständige JSON-Objekt {...}
static bool extractFirstJson(const string &in, string &out, size_t &consumed) {
    size_t start = in.find('{');
    if (start == string::npos) return false;
    int depth = 0;
    for (size_t i = start; i < in.size(); ++i) {
        if (in[i] == '{') ++depth;
        else if (in[i] == '}') --depth;
        if (depth == 0) {
            out = in.substr(start, i - start + 1);
            consumed = i + 1;
            return true;
        }
    }
    return false;
}

int main() {
    const char* server_ip   = "127.0.0.1";
    const int   server_port = 5555;

    // 1) Socket anlegen
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    // 2) Mit Server verbinden
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        cerr << "Invalid address\n"; 
        return 1;
    }
    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect"); 
        return 1;
    }

    // 3) Handshake: Server schickt "0" oder "1"
    char hbuf[32];
    int  hlen = read(sock, hbuf, sizeof(hbuf)-1);
    if (hlen <= 0) { cerr << "Handshake failed\n"; return 1; }
    hbuf[hlen] = '\0';
    int me = atoi(hbuf);
    cout << "I am player " << me << "\n";

    MinimaxAI ai(4);  // Suche bis Tiefe 4

    string leftover;
    char   buf[4096];

    while (true) {
        // a) GET-Request senden
        const char* getReq = R"("get")";
        if (write(sock, getReq, strlen(getReq)) < 0) {
            perror("write get");
            break;
        }

        // b) lies, bis wir ein komplettes JSON-Objekt haben
        string oneJson;
        size_t consumed = 0;
        while (!extractFirstJson(leftover, oneJson, consumed)) {
            int len = read(sock, buf, sizeof(buf));
            if (len <= 0) {
                cerr << "Server closed connection\n";
                goto END;
            }
            leftover.append(buf, len);
        }
        // c) entferne es aus dem Puffer
        leftover.erase(0, consumed);

        // d) parse JSON
        json resp;
        try {
            resp = json::parse(oneJson);
        } catch (const json::parse_error &e) {
            cerr << "JSON parse error: " << e.what() << "\nFragment: " << oneJson << "\n";
            break;
        }

        // e) warten, bis wirklich beide verbunden sind
        if (!resp.value("bothConnected", false)) {
            std::this_thread::sleep_for(100ms);
            continue;
        }

        // f) Board laden
        string fen = resp.at("board").get<string>();
        Board board;
        if (!board.loadFEN(fen)) {
            cerr << "Invalid FEN: " << fen << "\n";
            break;
        }

        // g) prüfen, ob wir am Zug sind
        char turn = resp.at("turn").get<string>()[0]; // 'r' oder 'b'
        bool myTurn = (me == 0 && turn == 'r') || (me == 1 && turn == 'b');
        if (!myTurn) 
            continue;

        // h) besten Zug berechnen
        Color side = (turn == 'r' ? Color::Red : Color::Blue);
        Move mv = ai.chooseMove(board, side);
        string mvStr = MoveGenerator::toString(mv);
        cout << "Sending move: " << mvStr << "\n";

        // i) für Guard-Moves "-1" anhängen
        if (count(mvStr.begin(), mvStr.end(), '-') == 1) {
            mvStr += "-1";
        }

        // j) rohes JSON-Stringliteral senden, z.B. "\"D7-D6-1\""
        string out = "\"" + mvStr + "\"";
        if (write(sock, out.c_str(), out.size()) < 0) {
            perror("write move");
            break;
        }

        // ▶ **ACK-Konsum**: Server schickt nach unserem Zug erneut den aktuellen Zustand.
        // Wir müssen es hier *einmal* weglesen, damit die nächste GET-Schleife
        // wirklich den neuen Stand empfängt.
        {
            // lege so lange Puffer an, bis wir ein {}-Objekt finden
            string ackJson;
            size_t ackConsumed = 0;
            leftover.clear();
            while (!extractFirstJson(leftover, ackJson, ackConsumed)) {
                int len = read(sock, buf, sizeof(buf));
                if (len <= 0) { cerr << "Server closed after move\n"; goto END; }
                leftover.append(buf, len);
            }
            // entferne ACK aus leftover
            leftover.erase(0, ackConsumed);
            // wir können ackJson ignorieren oder debug-printen
            // cout << "ACK: " << ackJson << "\n";
        }
    }

END:
    close(sock);
    return 0;
}
