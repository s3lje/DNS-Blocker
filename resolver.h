#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class Resolver {
    private:
        std::string upstreamIP_;
        uint16_t    port_; 
    public:
        explicit Resolver(const std::string& ip="1.1.1.1", uint16_t port = 53)
            : upstreamIP_(ip), port_(port){}

        std::vector<uint8_t> forward(const uint8_t* query, size_t qlen);
};

std::vector<uint8_t> Resolver::forward(const uint8_t* query, size_t qlen){
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return {};

    struct timeval tv{2, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port    = htons(port_);
    inet_pton(AF_INET, upstreamIP_.c_str(), &dest.sin_addr);

    ssize_t sent = sendto(sock, query, qlen, 0,
            reinterpret_cast<sockaddr*>(&dest), sizeof(dest));
    if (sent < 0) {
        close(sock); 
        return {};
    }

    std::vector<uint8_t> resp(512);
    ssize_t n = recvfrom(sock, resp.data(), resp.size(), 0, nullptr, nullptr);
    close(sock);

    if (n <= 0) return {};
    resp.resize(static_cast<size_t>(n));
    return resp;
}
