#pragma once
#include <cstdint>
#include <string>
#include <arpa/inet.h>  // htons / ntohs

struct DNSHeader {
    uint16_t id;        // Client-chosed ID
    uint16_t flags;     // QR, opcode, AA, TC, RD, RA, Z, RCODE
    uint16_t qdcount;   // Number of questions
    uint16_t ancount;   // Number of answers
    uint16_t nscount;   // Number of authority records
    uint16_t arcount;   // Number of additional records
};

struct DNSQuestion {
    std::string qname;
    uint16_t qtype;
    uint16_t qclass;
};

std::string readName(const uint8_t* buf, size_t buflen, size_t& offset) {
    std::string name;
    bool jumped = false;
    size_t maxJumps = 10, jumps = 0;

    while (offset < buflen) {
        uint8_t len = buf[offset];

        // Pointer (compression)
        if ((len & 0xC0) == 0xC0) {
            if (offset + 1 >= buflen) break;
            size_t ptr = ((len & 0x3F) << 8) | buf[offset + 1];
            if (!jumped) offset += 2;
            jumped = true;
            offset = ptr;
            if (++jumps > maxJumps) break;
            continue;
        }

        offset++;
        if (len == 0) break;  // root label
        if (!name.empty()) name += '.';
        if (offset + len > buflen) break;
        name.append(reinterpret_cast<const char*>(buf + offset), len);
        offset += len;
    }
    return name;
}

bool parseQuestion(const uint8_t* buf, size_t len, DNSQuestion& out){
    if (len < sizeof(DNSHeader) + 5) return false;
    size_t offset = sizeof(DNSHeader);
    out.qname = readName(buf, len, offset);
    if (out.qname.empty()) return false;

    if (offset + 4 > len) return false; 
    out.qtype = ntohs(*reinterpret_cast<const uint16_t*>(buf + offset)); offset += 2;
    out.qclass = ntohs(*reinterpret_cast<const uint16_t*>(buf + offset));
    return true; 
}

