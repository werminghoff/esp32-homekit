//
//  packet_data.cpp
//  CppCode
//
//  Created by Bruno Rigo Werminghoff on 05/03/20.
//  Copyright © 2020 Bruno Rigo Werminghoff. All rights reserved.
//

#include "packets.hpp"
#include "functions.hpp"

#include <sstream>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>

unsigned char* simple_message(const char* method, int id, size_t& length) {
    std::stringstream ss;
    ss << "{\"method\":\"" << method << "\",\"id\":" << id << "}";
    std::string str = ss.str();
    size_t buffer_size = (sizeof(unsigned char) * str.length()) + 1;
    unsigned char* buffer = (unsigned char*)malloc(buffer_size);
    ::memset(buffer, 0, buffer_size);
    if(buffer == nullptr) { return nullptr; }
    ::memcpy(buffer, str.c_str(), buffer_size-1);
    length = sizeof(unsigned char) * str.length();
    return buffer;
}

unsigned char* packet_find_me(int id, size_t& length) {
    return simple_message("find_me", id, length);
}

unsigned char* packet_clean(int id, size_t& length) {
    return simple_message("app_start", id, length);
}

unsigned char* packet_pause(int id, size_t& length) {
    return simple_message("app_pause", id, length);
}

unsigned char* packet_dock(int id, size_t& length) {
    return simple_message("app_charge", id, length);
}

unsigned char* packet_hello(size_t& length) {
    return hex2bin("21310020ffffffffffffffffffffffffffffffffffffffffffffffffffffffff", length);
}
