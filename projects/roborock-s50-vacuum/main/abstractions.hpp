//
//  abstractions.hpp
//  CppCode
//
//  Created by Bruno Rigo Werminghoff on 04/03/20.
//  Copyright © 2020 Bruno Rigo Werminghoff. All rights reserved.
//

#ifndef abstractions_hpp
#define abstractions_hpp

#include <sstream>
#include "packets.hpp"

#if __APPLE__
#define APP_LOG(format, ...) printf(format "\n", ## __VA_ARGS__)
#else
#include "esp_log.h"
#define APP_LOG(format, ...) ESP_LOGI(TAG, format, ## __VA_ARGS__)
#endif

#ifndef CBC_BLOCK_SIZE
#define CBC_BLOCK_SIZE 16
#endif

void md5(unsigned char* input, size_t len, unsigned char* output);
unsigned char* md5(unsigned char* input, size_t len);

void aes_128_cbc_encrypt(unsigned char* key, unsigned char* iv, size_t input_size, unsigned char* input, unsigned char* output);
void aes_128_cbc_decrypt(unsigned char* key, unsigned char* iv, size_t input_size, unsigned char* input, unsigned char* output);

st_packet* send(const unsigned char* data, size_t data_len, const char* hex_token, const char* ipv4, uint16_t port);

#endif /* abstractions_hpp */
