//
//  abstractions.cpp
//  CppCode
//
//  Created by Bruno Rigo Werminghoff on 04/03/20.
//  Copyright © 2020 Bruno Rigo Werminghoff. All rights reserved.
//

#include "abstractions.hpp"
#include "functions.hpp"

#include <sstream>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#if __APPLE__
#include <openssl/aes.h>

extern std::string md5(const char* data, size_t data_len);
extern std::string md5(const std::string str);

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#else

#include "esp_system.h"
#include "mbedtls/aes.h"
#include "mbedtls/md5.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define TAG "abstractions"

#endif

void md5(unsigned char* input, size_t len, unsigned char* output) {
    
#if __APPLE__
    std::stringstream ss;
    ss.write((char*)input, len);
    auto hash_string = md5(ss.str());
    
    size_t outlen;
    unsigned char* buffer = hex2bin((const char*)hash_string.c_str(), outlen);
    memcpy(output, buffer, outlen);
    free(buffer);
#else
    ESP_ERROR_CHECK(mbedtls_md5_ret((const unsigned char*)input, len, output));
#endif

}

unsigned char* md5(unsigned char* input, size_t len) {
#if __APPLE__
    unsigned char* output = (unsigned char*)malloc(16);
    md5(input, len, output);
    return output;
#else
    unsigned char* output = (unsigned char*)malloc(16);
    md5(input, len, output);
    return output;
#endif
}

void aes_128_cbc_encrypt(unsigned char* key, unsigned char* iv, size_t input_size, unsigned char* input, unsigned char* output) {
#if __APPLE__
    AES_KEY enc_key;
    AES_set_encrypt_key(key, 128, &enc_key);
    AES_cbc_encrypt(input, output, input_size, &enc_key, iv, AES_ENCRYPT);
#else
    mbedtls_aes_context aes_ctx;
    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_enc( &aes_ctx, key, 128);
    mbedtls_aes_crypt_cbc( &aes_ctx, MBEDTLS_AES_ENCRYPT, input_size, iv, input, output);
    mbedtls_aes_free(&aes_ctx);
#endif
}

void aes_128_cbc_decrypt(unsigned char* key, unsigned char* iv, size_t input_size, unsigned char* input, unsigned char* output) {
#if __APPLE__
    AES_KEY enc_key;
    AES_set_decrypt_key(key, 128, &enc_key);
    AES_cbc_encrypt(input, output, input_size, &enc_key, iv, AES_DECRYPT);
#else
    mbedtls_aes_context aes_ctx;
    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_dec( &aes_ctx, key, 128);
    mbedtls_aes_crypt_cbc( &aes_ctx, MBEDTLS_AES_DECRYPT, input_size, iv, input, output);
    mbedtls_aes_free(&aes_ctx);
#endif
}

st_packet* send(const unsigned char* data, size_t data_len, const char* hex_token, const char* ipv4, uint16_t port) {
    
    int sockfd;
    struct sockaddr_in servaddr;
    
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) < 0 ) {
        APP_LOG("Failed to create socket [%d]", sockfd);
        return nullptr;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(ipv4);
    
    ssize_t n = 0;
    socklen_t len = 0;
    
    if((n = sendto(sockfd, data, data_len, 0, (const struct sockaddr *) &servaddr, sizeof(servaddr))) != data_len) {
        APP_LOG("Failed to send data [%d]", (int)n);
        return nullptr;
    }

    unsigned char header[8];
    memset(header, 0, 8);
    len = 0;
    if((n = recvfrom(sockfd, (void *)header, 8, MSG_PEEK, (struct sockaddr *) &servaddr, &len)) == -1) {
        APP_LOG("Failed to receive header [%d]", (int)n);
        return nullptr;
    }
    
    uint16_t size = (*(uint16_t*)(&header[3]));
    unsigned char packet_data[size];
    memset(packet_data, 0, size);
    len = 0;
    if((n = recvfrom(sockfd, (void *)packet_data, size, 0, (struct sockaddr *) &servaddr, &len)) == -1) {
        APP_LOG("Failed to receive body [%d]", (int)n);
        return nullptr;
    }
    
    st_packet* packet = new st_packet;
    packet_parse((const unsigned char*)packet_data, (unsigned char*)hex_token, *packet);
    
    close(sockfd);
    
    return packet;
}

