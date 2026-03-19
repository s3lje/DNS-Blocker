#include <cassert>
#include <cstring>
#include <cstdio>
#include "../dns.h"

void test_header_size(){
    // DNS spec requires exactly 12 bytes
    assert(sizeof(DNSHeader) == 12);
    printf("  PASS  header is 12 bytes\n");
}

void test_header_field_layout(){
    uint8_t raw[12] = {
        0x12, 0x34,   // id
        0x01, 0x00,   // flags (RD=1)
        0x00, 0x01,   // qdcount=1
        0x00, 0x00,   // ancount
        0x00, 0x00,   // nscount
        0x00, 0x00    // arcount
    };
    auto* h = reinterpret_cast<DNSHeader*>(raw);
    assert(ntohs(h->id)      == 0x1234);
    assert(ntohs(h->flags)   == 0x0100);
    assert(ntohs(h->qdcount) == 1);
    printf("  PASS  header fields parse correctly from raw bytes\n");
}

void test_readName_simple(){
    uint8_t buf[] = {
        7,'e','x','a','m','p','l','e',
        3,'c','o','m',
        0
    };
    size_t offset = 0;
    std::string name = readName(buf, sizeof(buf), offset);
    assert(name == "example.com");
    assert(offset == 13);  // consumed all 13 bytes
    printf("  PASS  simple name 'example.com'\n");
}

void test_readName_single_label() {
    uint8_t buf[] = { 9,'l','o','c','a','l','h','o','s','t', 0 };
    size_t offset = 0;
    assert(readName(buf, sizeof(buf), offset) == "localhost");
    printf("  PASS  single label 'localhost'\n");
}

void test_readName_pointer_compression(){
    uint8_t buf[] = {
        // offset 0: example.com
        7,'e','x','a','m','p','l','e', 3,'c','o','m', 0,
        // offset 13: www + pointer to 0
        3,'w','w','w', 0xC0, 0x00
    };
    size_t offset = 13;
    std::string name = readName(buf, sizeof(buf), offset);
    assert(name == "www.example.com");
    printf("  PASS  compressed name 'www.example.com'\n");
}

void test_readName_truncated_is_safe(){
    uint8_t buf[] = { 7, 'e', 'x' };  // claims 7 bytes but only 2 follow
    size_t offset = 0;
    std::string name = readName(buf, sizeof(buf), offset);
    
    (void)name;
    printf("  PASS  truncated buffer doesn't crash\n");
}

int main() {
    printf("=== dns.h: DNSHeader ===\n");
    test_header_size();
    test_header_field_layout();
    printf("All header tests passed.\n\n");

    printf("=== dns.h: readName ===\n");
    test_readName_simple();
    test_readName_single_label();
    test_readName_pointer_compression();
    test_readName_truncated_is_safe();
    printf("All readName tests passed.\n\n");
}
