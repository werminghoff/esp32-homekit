//
//  functions.cpp
//  CppCode
//
//  Created by Bruno Rigo Werminghoff on 04/03/20.
//  Copyright © 2020 Bruno Rigo Werminghoff. All rights reserved.
//

#include "functions.hpp"
#include "abstractions.hpp"
#include "config.h"

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

size_t size_padding(size_t original_size) {
    uint8_t padding_size = (uint8_t)(CBC_BLOCK_SIZE - (original_size % (int)CBC_BLOCK_SIZE));
    if(padding_size == CBC_BLOCK_SIZE) { padding_size = 0; }
    return original_size + padding_size;
}

void decrypt_packet(st_packet& packet, unsigned char* key, unsigned char* iv) {
    if(packet.data_size > 0 && packet.data && key && iv) {
        unsigned char output[1024];
        aes_128_cbc_decrypt(key, iv, packet.data_size, packet.data, output);
        memcpy(packet.data, output, packet.data_size);
    }
}

void debug_buffer(unsigned char* buffer, size_t len, const char* name) {
    char* debug = bin2hex(buffer, len);
    printf("%s = %s\n", debug, name);
    free(debug);
}

void packet_parse(const unsigned char* raw_packet, unsigned char* token, st_packet& output) {
    output.magic = ((raw_packet[0] & 0xFF) << 8) | (raw_packet[1] & 0xFF);
    output.packet_length = ((raw_packet[2] & 0xFF) << 8) | (raw_packet[3] & 0xFF);
    output.unknown1 = ((raw_packet[4] & 0xFF) << 24) | ((raw_packet[5] & 0xFF) << 16) | ((raw_packet[6] & 0xFF) << 8) | (raw_packet[7] & 0xFF);
    output.unknown2 = ((raw_packet[8] & 0xFF) << 24) | ((raw_packet[9] & 0xFF) << 16) | ((raw_packet[10] & 0xFF) << 8) | (raw_packet[11] & 0xFF);
    output.timestamp = ((raw_packet[12] & 0xFF) << 24) | ((raw_packet[13] & 0xFF) << 16) | ((raw_packet[14] & 0xFF) << 8) | (raw_packet[15] & 0xFF);
    memcpy(output.checksum, &raw_packet[16], 16);
    
    if(output.packet_length > 32) {
        // data packet
        output.data_size = output.packet_length-32;
        output.data = (unsigned char*)malloc(output.data_size+1);
        memcpy(output.data, &raw_packet[32], output.data_size);
        output.data[output.data_size] = 0x00;
        
        unsigned char key[16];
        unsigned char iv[16];
        generate_key_and_iv((const char*)token, (unsigned char*)key, (unsigned char*)iv);
        decrypt_packet(output, (unsigned char*)key, (unsigned char*)iv);
        
        uint8_t padding = output.data[output.data_size-1];
        if(padding < output.data_size) {
            uint8_t padded_buffer[padding];
            memset(padded_buffer, padding, padding);
            unsigned char* padding_ptr = output.data + (output.data_size-padding);
            if(memcmp((unsigned char*)padding_ptr, padded_buffer, (int)padding) == 0){
                memset(padding_ptr, 0, padding);
            }
        }
        
    } else {
        // hello packet
        output.data = nullptr;
        output.data_size = 0;
    }
}

void buffer_write(unsigned char* buffer, uint8_t* data, size_t len, size_t& inout_index, bool inverted = false) {
  for(int i=0;i<len;i++) {
      if(inverted){
        ::memcpy(&(buffer[inout_index++]), &data[(len-i)-1], 1);
      } else {
        ::memcpy(&(buffer[inout_index++]), &data[i], 1);
      }
  }
}

template<typename T>
void buffer_write(unsigned char* buffer, T value, size_t& inout_index) {
  T copy = value;
  buffer_write(buffer, (uint8_t*)&copy, sizeof(copy), inout_index, true);
}

unsigned char* packet_build(const st_packet& packet, const char* token, size_t& out_packet_len) {

    // Calculate sizes including AES padding
    size_t total_data_size = size_padding(packet.data_size);
    uint8_t padding_size = total_data_size - packet.data_size;
    
    unsigned char* encrypted_data = (unsigned char*)malloc(total_data_size);
    if(packet.data && packet.data_size > 0) {
        
        unsigned char key[16];
        unsigned char iv[16];
        generate_key_and_iv(token, key, iv);
        memcpy(encrypted_data, packet.data, packet.data_size);
        memset(encrypted_data+packet.data_size, padding_size, padding_size);
        aes_128_cbc_encrypt(key, iv, total_data_size, encrypted_data, encrypted_data);
    }
    
    size_t base_size = sizeof(packet.magic);
    base_size += sizeof(packet.packet_length);
    base_size += sizeof(packet.unknown1);
    base_size += sizeof(packet.unknown2);
    base_size += sizeof(packet.timestamp);
    base_size += sizeof(packet.checksum);
    base_size += packet.data_size;
    size_t total_packet_size = base_size + padding_size;
    
    out_packet_len = total_packet_size;
    unsigned char* data_buffer = (unsigned char*)malloc(total_packet_size);
    
    // Write headers
    size_t index = 0;
    buffer_write<uint16_t>(data_buffer, packet.magic, index);
    buffer_write<uint16_t>(data_buffer, total_packet_size, index);
    buffer_write<uint32_t>(data_buffer, packet.unknown1, index);
    buffer_write<uint32_t>(data_buffer, packet.unknown2, index);
    buffer_write<uint32_t>(data_buffer, packet.timestamp, index);
    
    // Write (temporarily) the original token as the checksum
    size_t md5_index = index;
    memcpy((unsigned char*)packet.checksum, (unsigned char*)hex2bin(token), 16);
    buffer_write(data_buffer, (uint8_t*)packet.checksum, sizeof(packet.checksum), index);
    
    // Write the encrypted data
    buffer_write(data_buffer, (uint8_t*)encrypted_data, total_data_size, index);
    
    // Apply the final checksum
    md5(data_buffer, total_packet_size, &data_buffer[md5_index]);
    
    free(encrypted_data);
    return data_buffer;
}


