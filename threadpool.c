#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "threadpool.h"
#include <semaphore.h>

#define QUEUE_SIZE 10   //WORKQUEUE의 크기 : 10

#define NUMBER_OF_THREADS 3 //TreadPool에서 생성한 thread의 수 : 3
#define TRUE 1


typedef struct{
    void (*function)(void *p);
    void *data;
}task;

//the work queue
task workqueue[QUEUE_SIZE];
task dequeue();
int enqueue(task t);

int front; //queue의 앞
int rear; //queue의 뒤
int count; //queue의 요소 개수

//the worker threads
pthread_t tid[NUMBER_OF_THREADS];
sem_t sem; // counter = 0;
pthread_mutex_t mutex;

void pool_init(void){
    //mutex 초기화
    pthread_mutex_init(&mutex,NULL);
    //세마포 sem 초기화 (count =0)
    sem_init(&sem, 0, 0);
    //NUMBER_OF_THREADS 만큼 스레드 생성
    for(int i=0; i<NUMBER_OF_THREADS; i++ ){
        pthread_create(&tid[i],NULL, worker, NULL);
    }
}

void* worker(void*param){
    while(TRUE){
        printf("%lu awaiting a task....\n", pthread_self());
        //세마포 sem에 대한 P연산 코드 삽입
        sem_wait(&sem);
        printf("%lu got a task to do\n", pthread_self());
        task work = dequeue();
        execute(work.function, work.data);
        usleep(10000);
        pthread_testcancel();
    }
    pthread_exit(0);
}

void execute(void(*func)(void *param), void*param){
    //func을 param을 인자로하여 실행하는 코드 삽입
    (*func)(param);
}

task dequeue(){ //work queue로부터 작업 하나 가져오는 dequeue 정의
    task t;
    pthread_mutex_lock(&mutex);
    t = workqueue[front];
    front = (front +1) % QUEUE_SIZE;
    pthread_mutex_unlock(&mutex);
    return t;
}

int pool_submit(void (*func)(void*param), void*param){
    task t;
    //t에 func와 param 정보 삽입하기
    t.function = func;
    t.data = param;
    if (enqueue(t))
    {
        sem_post(&sem);
        return 0;
    }
    return 1;
}

int enqueue(task t){    //큐에 정보 넣기
    int ret_val = 0;
    pthread_mutex_lock(&mutex); //배타적 접근을 위한 mutex lock
    if(count < QUEUE_SIZE){
        workqueue[rear] = t;
        rear = (rear+1) % QUEUE_SIZE;
        count ++;
        ret_val = 1;
    }
    pthread_mutex_unlock(&mutex);
    return ret_val;
}

void pool_shutdown(void){   //pthread_cancel 수행
    for(int i=0; i<NUMBER_OF_THREADS; i++){
        pthread_cancel(tid[i]);
    }
}