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
}
