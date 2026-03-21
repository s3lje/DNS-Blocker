#include <iostream>
#include <csignal>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "dns.h"
#include "blocklist.h"
#include "resolver.h"

static volatile bool running = true;
void onSignal(int) { running = false; };

int main(int argc, char* argv[]){
    const char* blPath   = argc > 1 ? argv[1] : "blocklists/hosts.txt"; 
    const char* upstream = argc > 2 ? argv[2] : "1.1.1.1";

    Blocklist bl;
    if (!bl.load(blPath))
        return EXIT_FAILURE;

    Resolver resolver(upstream);


    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0){
        perror("socket");
        return EXIT_FAILURE;
    }

    int yes = 1; 
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(53);

    if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0){
        perror("bind - try running with sudo...\n");
        close(sock);
        return EXIT_FAILURE;
    }

    
    signal(SIGINT,  onSignal);
    signal(SIGTERM, onSignal);
    
    std::cout << "dns-blocker running on :53 upstream=" << upstream << std::endl;

    // Main recieving loop
    uint8_t buf[512];
    while(running) {
        sockaddr_in client{};
        socklen_t clen = sizeof(client);
        ssize_t n = recvfrom(sock, buf, sizeof(buf), 0,
                             reinterpret_cast<sockaddr*>(&client), &clen);
        if (n <= 0) continue;

        DNSQuestion q;
        if (parseQuestion(buf, n, q)) continue;

        std::vector<uint8_t> response; 
    
        if (bl.isBlocked(q.qname)){
            std::cout << "[BLOCK]  "<< q.qname << std::endl;
            response = makeNXDomain(buf, static_cast<size_t>(n));
        } else {
            response = resolver.forward(buf, static_cast<size_t>(n));
            if (response.empty()){
                std::cerr << "[ERROR]  upstream timeout for " << q.qname << std::endl;
                response = makeNXDomain(buf, static_cast<size_t>(n));
            } else {
                std::cout << "[ALLOW]  " << q.qname << std::endl;
            }
        }
        sendto(sock, response.data(), response.size(), 0,
                reinterpret_cast<sockaddr*>(&client), clen);
    }

    close(sock);
    std::cout << "Stopped.\n";
    return 0;
}
