//
//  packet_data.hpp
//  CppCode
//
//  Created by Bruno Rigo Werminghoff on 05/03/20.
//  Copyright © 2020 Bruno Rigo Werminghoff. All rights reserved.
//

#ifndef packet_data_hpp
#define packet_data_hpp

#include <sstream>
#include <stdio.h>

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

unsigned char* simple_message(const char* method, int id, size_t& length);
unsigned char* packet_hello(size_t& length);
unsigned char* packet_find_me(int id, size_t& length);
unsigned char* packet_clean(int id, size_t& length);
unsigned char* packet_pause(int id, size_t& length);
unsigned char* packet_dock(int id, size_t& length);

#endif /* packet_data_hpp */
