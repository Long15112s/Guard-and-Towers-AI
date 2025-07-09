#pragma once
#include <SFML/Network.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <optional>

namespace pages {

class NetworkConnector {
public:
    NetworkConnector(const std::string& server, unsigned port);
    bool connect();
    int  playerId() const;
    void sendGet();
    void sendMove(const std::string& moveStr);
    std::optional<nlohmann::json> receive();

private:
    sf::TcpSocket   _socket;
    std::string     _srv;
    unsigned        _port;
    std::string     _buffer;
    int             _me = -1;
};

} // namespace pages
