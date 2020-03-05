//
//  functions.cpp
//  CppCode
//
//  Created by Bruno Rigo Werminghoff on 04/03/20.
//  Copyright © 2020 Bruno Rigo Werminghoff. All rights reserved.
//

#include "functions.hpp"
#include "abstractions.hpp"

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>

unsigned char* hex2bin(const char* input, size_t& out_length) {
    size_t len = strlen(input);
    unsigned char* buffer = (unsigned char*)malloc((size_t)(len/2));
    
    const char* ptr = input;
    int one_byte = 0;
    int idx = 0;
    while(sscanf(ptr, "%02x", &one_byte) == 1) {
        ptr++;ptr++;
        buffer[idx] = (unsigned char)(one_byte & 0xFF);
        idx++;
    }
    out_length = idx;
    return buffer;
}

char* bin2hex(unsigned char* binary, size_t len, size_t& out_length) {
    out_length = (len*2);
    char* buffer = (char*)malloc(out_length+1);
    ::memset(buffer, 0, out_length+1);
    for(int i=0;i<len;i++){
        sprintf(buffer+strlen(buffer), "%02x", binary[i]);
    }
    return buffer;
}

unsigned char* hex2bin(const char* input) {
    size_t unused;
    return hex2bin(input, unused);
}

char* bin2hex(unsigned char* binary, size_t len) {
    size_t unused;
    return bin2hex(binary, len, unused);
}

void generate_key_and_iv(const char* text_token, unsigned char* out_key, unsigned char* out_iv) {
    
    size_t len;
    unsigned char* token = hex2bin(text_token, len);
    auto enc_key = md5((unsigned char*)token, len);
    
    char iv_source[(len*4) + 1];
    ::memset(iv_source, 0, sizeof(iv_source));
    
    for(int i=0; i < 16; i++) {
        sprintf(iv_source + strlen(iv_source), "%02x", (int)(enc_key[i] & 0xFF));
    }
    
    for(int i=0; i < len; i++) {
        sprintf(iv_source + strlen(iv_source), "%02x", (int)(token[i] & 0xFF));
    }
    
    auto iv_source_bin = hex2bin(iv_source, len);
    auto enc_iv = md5(iv_source_bin, len);
    
    ::memcpy(out_key, enc_key, 16);
    ::memcpy(out_iv, enc_iv, 16);
    
    free(iv_source_bin);
    free(enc_iv);
    free(enc_key);
    free(token);
}

void packet_parse(const unsigned char* raw_packet, st_packet& output) {
    output.magic = ((raw_packet[0] & 0xFF) << 8) | (raw_packet[1] & 0xFF);
    output.packet_length = ((raw_packet[2] & 0xFF) << 8) | (raw_packet[3] & 0xFF);
    output.unknown1 = ((raw_packet[4] & 0xFF) << 24) | ((raw_packet[5] & 0xFF) << 16) | ((raw_packet[6] & 0xFF) << 8) | (raw_packet[7] & 0xFF);
    output.unknown2 = ((raw_packet[8] & 0xFF) << 24) | ((raw_packet[9] & 0xFF) << 16) | ((raw_packet[10] & 0xFF) << 8) | (raw_packet[11] & 0xFF);
    output.timestamp = ((raw_packet[12] & 0xFF) << 24) | ((raw_packet[13] & 0xFF) << 16) | ((raw_packet[14] & 0xFF) << 8) | (raw_packet[15] & 0xFF);
    memcpy(output.checksum, &raw_packet[16], 16);
    
    if(output.packet_length > 32) {
        output.data_size = output.packet_length-32;
        output.data = (unsigned char*)malloc(output.data_size);
        memcpy(output.data, &raw_packet[32], output.data_size);
    } else {
        output.data = nullptr;
        output.data_size = 0;
    }
}

void decrypt_packet(st_packet& packet, unsigned char* key, unsigned char* iv) {
    if(packet.data_size > 0 && packet.data && key && iv) {
        aes_128_cbc_decrypt(key, iv, packet.data_size, packet.data, packet.data);
    }
}

unsigned char* packet_build(const st_packet& packet, size_t& out_data) {
    return nullptr;
}
