//
//  abstractions.cpp
//  CppCode
//
//  Created by Bruno Rigo Werminghoff on 04/03/20.
//  Copyright © 2020 Bruno Rigo Werminghoff. All rights reserved.
//

#include "abstractions.hpp"
#include "functions.hpp"

#if __APPLE__
#include <openssl/aes.h>

extern std::string md5(const char* data, size_t data_len);
extern std::string md5(const std::string str);

#else
#include "esp_system.h"
#include "mbedtls/aes.h"
#include "mbedtls/md5.h"
#endif

void md5(unsigned char* input, size_t len, unsigned char* output) {
    {
//        printf("\n");
//        char* debug = bin2hex(input, len);
//        printf("hashing %s\n", debug);
//        free(debug);
    }
    
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
    
    {
//        char* md5hash = bin2hex(output, 16);
//        printf("result %s\n", md5hash);
//        printf("\n");
//        free(md5hash);
    }
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
