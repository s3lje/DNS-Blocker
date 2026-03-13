#pragma once
#include <cstdint>
#include <arpa/inet.h>  // htons / ntohs

struct DNSHeader {
    uint16_t id;        // Client-chosed ID
    uint16_t flags;     // QR, opcode, AA, TC, RD, RA, Z, RCODE
    uint16_t qdcount;   // Number of questions
    uint16_t ancount;   // Number of answers
    uint16_t nscount;   // Number of authority records
    uint16_t arcount;   // Number of additional records
};

