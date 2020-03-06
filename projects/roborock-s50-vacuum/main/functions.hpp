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
#include "packets.hpp"

void packet_parse(const unsigned char* raw_packet, unsigned char* token, st_packet& output);
unsigned char* packet_build(const st_packet& packet, const char* token, size_t& out_packet_len);

size_t size_padding(size_t original_size);

unsigned char* hex2bin(const char* token, size_t& out_length);
unsigned char* hex2bin(const char* token);
char* bin2hex(unsigned char* binary, size_t len, size_t& out_length);
char* bin2hex(unsigned char* binary, size_t len);
void generate_key_and_iv(const char* text_token, unsigned char* out_key, unsigned char* out_iv);

#endif /* functions_hpp */
