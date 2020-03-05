//
//  tests.cpp
//  CppCode
//
//  Created by Bruno Rigo Werminghoff on 04/03/20.
//  Copyright © 2020 Bruno Rigo Werminghoff. All rights reserved.
//

#include <sstream>
#include <memory>
#include "abstractions.hpp"
#include "functions.hpp"

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#define TAG "tests"

void test_bin2hex() {
    
    {
        unsigned char buffer[16];
        memset(buffer, 0, 16);
        size_t out;
        auto hex = bin2hex(buffer, 16, out);
        assert(strcmp(hex, "00000000000000000000000000000000") == 0);
        free(hex);
    }
    
    {
        unsigned char buffer[16];
        memset(buffer, 1, 16);
        size_t out;
        auto hex = bin2hex(buffer, 16, out);
        assert(strcmp(hex, "01010101010101010101010101010101") == 0);
        free(hex);
    }
    
    {
        unsigned char buffer[16];
        memset(buffer, 233, 16);
        size_t out;
        auto hex = bin2hex(buffer, 16, out);
        assert(strcmp(hex, "e9e9e9e9e9e9e9e9e9e9e9e9e9e9e9e9") == 0);
        free(hex);
    }
    
}

void test_hex2bin() {
    
    {
        char* hex_string = (char*)"00000000000000000000000000000000";
        size_t len;
        unsigned char* binary = hex2bin(hex_string, len);
        assert(len == 16);
        unsigned char buffer[16];
        memset(buffer, 0, 16);
        assert(memcmp(binary, buffer, 16) == 0);
        free(binary);
    }
    
    {
        char* hex_string = (char*)"01010101010101010101010101010101";
        size_t len;
        unsigned char* binary = hex2bin(hex_string, len);
        assert(len == 16);
        unsigned char buffer[16];
        memset(buffer, 1, 16);
        assert(memcmp(binary, buffer, 16) == 0);
        free(binary);
    }
    
    {
        char* hex_string = (char*)"e9e9e9e9e9e9e9e9e9e9e9e9e9e9e9e9";
        size_t len;
        unsigned char* binary = hex2bin(hex_string, len);
        assert(len == 16);
        unsigned char buffer[16];
        memset(buffer, 233, 16);
        assert(memcmp(binary, buffer, 16) == 0);
        free(binary);
    }
    
}

void test_md5() {
    
    {
        char* expected = (char*)"d41d8cd98f00b204e9800998ecf8427e";
        unsigned char binary[16];
        char* text = (char*)"";
        md5((unsigned char*)text, strlen(text), binary);
        char* hex = (char*)bin2hex(binary, 16);
        assert(strcmp(expected, hex) == 0);
        free(hex);
    }
    
    {
        char* expected = (char*)"9e107d9d372bb6826bd81d3542a419d6";
        unsigned char binary[16];
        char* text = (char*)"The quick brown fox jumps over the lazy dog";
        md5((unsigned char*)text, strlen(text), binary);
        char* hex = (char*)bin2hex(binary, 16);
        assert(strcmp(expected, hex) == 0);
        free(hex);
    }
    
    {
        char* expected = (char*)"2cd95c5d54b689c6538264c3e8daaa27";
        unsigned char binary[16];
        char* text = (char*)"111111111111111111111111";
        md5((unsigned char*)text, strlen(text), binary);
        char* hex = (char*)bin2hex(binary, 16);
        assert(strcmp(expected, hex) == 0);
        free(hex);
    }
}

void test_token_to_key_iv() {
    
    {
        char* token = (char*)"787664594b6c3255544f6c75386f6d72";
        char* expected_key = (char*)"415a1ead769110aa2a85b79340836bad";
        char* expected_iv = (char*)"78a5448dd9201aa2ff4cac001e124cac";
        
        unsigned char enc_key[16];
        unsigned char enc_iv[16];
        ::memset(enc_key, 0, sizeof(enc_key));
        ::memset(enc_iv, 0, sizeof(enc_iv));
        
        generate_key_and_iv(token, enc_key, enc_iv);
        
        {
            char* hex_key = (char*)bin2hex(enc_key, 16);
            assert(strcmp(expected_key, hex_key) == 0);
            free(hex_key);
        }
        
        {
            char* hex_iv = (char*)bin2hex(enc_iv, 16);
            assert(strcmp(expected_iv, hex_iv) == 0);
            free(hex_iv);
        }
    }
    
    {
        char* token = (char*)"726a6231787a6b56454e636574453367";
        char* expected_key = (char*)"9ffd97bc320f4d1e98dc2cbdd3f42117";
        char* expected_iv = (char*)"9463a829e6ddca1f6bbe8ff936f1693f";
        
        unsigned char enc_key[16];
        unsigned char enc_iv[16];
        ::memset(enc_key, 0, sizeof(enc_key));
        ::memset(enc_iv, 0, sizeof(enc_iv));
        
        generate_key_and_iv(token, enc_key, enc_iv);
        
        {
            char* hex_key = (char*)bin2hex(enc_key, 16);
            assert(strcmp(expected_key, hex_key) == 0);
            free(hex_key);
        }
        
        {
            char* hex_iv = (char*)bin2hex(enc_iv, 16);
            assert(strcmp(expected_iv, hex_iv) == 0);
            free(hex_iv);
        }
    }
    
    {
        char* token = (char*)"64666152516a6d614b51746574794653";
        char* expected_key = (char*)"7fd47e8d15cce27b1941c652c655eff7";
        char* expected_iv = (char*)"e35a58cba8abfacb669a653376dad958";
        
        unsigned char enc_key[16];
        unsigned char enc_iv[16];
        ::memset(enc_key, 0, sizeof(enc_key));
        ::memset(enc_iv, 0, sizeof(enc_iv));
        
        generate_key_and_iv(token, enc_key, enc_iv);
        
        {
            char* hex_key = (char*)bin2hex(enc_key, 16);
            assert(strcmp(expected_key, hex_key) == 0);
            free(hex_key);
        }
        
        {
            char* hex_iv = (char*)bin2hex(enc_iv, 16);
            assert(strcmp(expected_iv, hex_iv) == 0);
            free(hex_iv);
        }
    }
    
    {
        char* token = (char*)"494964464146465154337779414e6954";
        char* expected_key = (char*)"6b76f18aab46f3e6d7912383677230d0";
        char* expected_iv = (char*)"75ae5cab714383ca290528f350d675fd";
        
        unsigned char enc_key[16];
        unsigned char enc_iv[16];
        ::memset(enc_key, 0, sizeof(enc_key));
        ::memset(enc_iv, 0, sizeof(enc_iv));
        
        generate_key_and_iv(token, enc_key, enc_iv);
        
        {
            char* hex_key = (char*)bin2hex(enc_key, 16);
            assert(strcmp(expected_key, hex_key) == 0);
            free(hex_key);
        }
        
        {
            char* hex_iv = (char*)bin2hex(enc_iv, 16);
            assert(strcmp(expected_iv, hex_iv) == 0);
            free(hex_iv);
        }
    }
    
}

void test_aes() {
    
    // Encrypt
    {
        char* input = (char*)malloc(1024);
        memset(input, 0, 1024);
        strcpy(input, "The quick brown fox jumps over the lazy dog");
        char* key = (char*)"6b76f18aab46f3e6d7912383677230d0";
        char* iv = (char*)"75ae5cab714383ca290528f350d675fd";
        char* expected_output = (char*)"4e7086a8c311cb96143b6072df207e449a36cc556363d86f50b96c3a7a56df9e6af628594309ea13410936dc24017b8b";
        
        size_t padded_size = strlen(expected_output)/2;
        
        unsigned char* bin_key = hex2bin(key);
        unsigned char* bin_iv = hex2bin(iv);
        
        unsigned char* output = (unsigned char*)malloc(1024);
        aes_128_cbc_encrypt(bin_key, bin_iv, padded_size, (unsigned char*)input, (unsigned char*)output);
        
        char* hex_output = bin2hex(output, padded_size);
        assert(strcmp(expected_output, hex_output) == 0);
        
        free(output);
        free(input);
        free(hex_output);
        free(bin_key);
        free(bin_iv);
    }
    
    // Decrypt
    {
        char* input = (char*)malloc(1024);
        memset(input, 0, 1024);
        strcpy(input, "4e7086a8c311cb96143b6072df207e449a36cc556363d86f50b96c3a7a56df9e6af628594309ea13410936dc24017b8b");
        char* key = (char*)"6b76f18aab46f3e6d7912383677230d0";
        char* iv = (char*)"75ae5cab714383ca290528f350d675fd";
        char* expected_output = (char*)"The quick brown fox jumps over the lazy dog";
        
        size_t padded_size = strlen(input);
        
        unsigned char* bin_input = hex2bin(input);
        unsigned char* bin_key = hex2bin(key);
        unsigned char* bin_iv = hex2bin(iv);
        
        unsigned char* output = (unsigned char*)malloc(1024);
        aes_128_cbc_decrypt(bin_key, bin_iv, padded_size, bin_input, output);
        assert(strcmp(expected_output, (char*)output) == 0);
        
        free(output);
        free(input);
        free(bin_input);
        free(bin_key);
        free(bin_iv);
    }
    
}

void test_packet_parse() {
    
    {
        char* hex_packet = (char*)"21310020000000000f9d73765e607dd1ffffffffffffffffffffffffffffffff";
        unsigned char* bin_packet = hex2bin(hex_packet);
        st_packet packet;
        packet_parse(bin_packet, packet);
        
        unsigned char expected_checksum[16];
        unsigned char expected_data[16];
        memset(expected_checksum, -1, 16);
        memset(expected_data, -1, 16);
        
        assert(packet.magic == 0x2131);
        assert(packet.packet_length == 0x0020);
        assert(packet.unknown1 == 0x00000000);
        assert(packet.unknown2 == 0x0f9d7376);
        assert(packet.timestamp == 0x5e607dd1);
        assert(memcmp(packet.checksum, expected_checksum, 16) == 0);
        assert(packet.data == nullptr);
        assert(packet.data_size == 0);
        
    }
    
}

void run_tests() {
    APP_LOG("Testing bin2hex...");
    test_bin2hex();
    APP_LOG("Testing hex2bin...");
    test_hex2bin();
    APP_LOG("Testing md5...");
    test_md5();
    APP_LOG("Testing aes...");
    test_aes();
    APP_LOG("Testing token to key and iv...");
    test_token_to_key_iv();
    APP_LOG("Testing packet parser...");
    test_packet_parse();
    
    APP_LOG("All tests passed! :-)");
}
