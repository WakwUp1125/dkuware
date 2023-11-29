#ifndef CRYPTO
#define CRYPTO

#include <stdio.h>

typedef struct _INFO { // 각 파일 정보 구조체 선언 -> JPG, PDF 구조체 변수 2개 선언(각 스레드 함수 매개변수로 전달) 
    char password[17]; // main 함수 인자로 받은 password를 입력
    int sum; // 각 파일의 개수
    char SIG[5]; // 각 파일 확인용 시그니처(".JPG", ".PDF")
    char **fileName; // 각 파일 이름 배열
    FILE **fpArr; // 각 파일 포인터 배열
} INFO, *PINFO;

void dump_hex(char *label, unsigned char buf[], int size);

void aes_128_encrypt(PINFO param);

void aes_128_decrypt(PINFO param);

#endif