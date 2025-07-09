#include "NetworkConnector.hpp"
using json = nlohmann::json;

namespace pages {

NetworkConnector::NetworkConnector(const std::string& srv, unsigned port)
: _srv(srv), _port(port)
{}

bool NetworkConnector::connect() {
    if (_socket.connect(_srv, _port) != sf::Socket::Done)
        return false;
    // handshake
    char tmp[32]; std::size_t rec;
    if (_socket.receive(tmp, sizeof(tmp), rec) != sf::Socket::Done)
        return false;
    tmp[rec] = '\0';
    _me = std::atoi(tmp);
    return true;
}

int NetworkConnector::playerId() const { return _me; }

void NetworkConnector::sendGet() {
    const std::string r = R"("get")";
    _socket.send(r.c_str(), r.size());
}

void NetworkConnector::sendMove(const std::string& m) {
    std::string out = "\"" + m + "\"";
    _socket.send(out.c_str(), out.size());
}

std::optional<json> NetworkConnector::receive() {
    char tmp[4096]; std::size_t len;
    if (_socket.receive(tmp, sizeof(tmp), len) != sf::Socket::Done)
        return std::nullopt;
    _buffer.append(tmp, len);

    int depth = 0;
    auto start = _buffer.find('{');
    for (size_t i = start; i < _buffer.size(); ++i) {
        if (_buffer[i]=='{') ++depth;
        else if (_buffer[i]=='}') --depth;
        if (depth==0 && start!=std::string::npos) {
            auto piece = _buffer.substr(start, i-start+1);
            _buffer.erase(0, i+1);
            try { return json::parse(piece); }
            catch(...)  { return std::nullopt; }
        }
    }
    return std::nullopt;
}

} // namespace pages
