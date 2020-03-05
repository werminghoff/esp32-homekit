//
//  functions.hpp
//  CppCode
//
//  Created by Bruno Rigo Werminghoff on 04/03/20.
//  Copyright © 2020 Bruno Rigo Werminghoff. All rights reserved.
//

#ifndef functions_hpp
#define functions_hpp

#include <sstream>

typedef struct {
    uint16_t magic;
    uint16_t packet_length;
    uint32_t unknown1;
    uint32_t unknown2;
    uint32_t timestamp;
    unsigned char checksum[16];
    unsigned char* data;
    size_t data_size;
} st_packet;

void packet_parse(const unsigned char* raw_packet, st_packet& output);
unsigned char* packet_build(const st_packet& packet, size_t& out_data);

unsigned char* hex2bin(const char* token, size_t& out_length);
unsigned char* hex2bin(const char* token);
char* bin2hex(unsigned char* binary, size_t len, size_t& out_length);
char* bin2hex(unsigned char* binary, size_t len);
void generate_key_and_iv(const char* text_token, unsigned char* out_key, unsigned char* out_iv);

#endif /* functions_hpp */
