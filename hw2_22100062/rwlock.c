#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

// Reader-Writer Lock 구조체 정의
// 공정성(저자 우선)을 위해 writers 카운터와 rlock 세마포어 추가
typedef struct _rwlock_t {
    sem_t mutex;      // readers, writers 변수 접근을 위한 뮤텍스
    sem_t writelock;  // 저자 스레드의 배타적 접근을 위한 세마포어
    sem_t rlock;      // 저자가 대기 중일 때 독자를 막기 위한 세마포어
    int readers;      // 현재 읽고 있는 독자 수
    int writers;      // 현재 대기 중인 저자 수
} rwlock_t;

// 스레드에 전달될 인자 구조체
typedef struct _thread_arg_t {
    int id;
    char type;
    int process_time;
    rwlock_t *rw;
    struct timeval *start_time;
} thread_arg_t;

// 함수 프로토타입
void rwlock_init(rwlock_t *rw);
void rwlock_acquire_readlock(rwlock_t *rw);
void rwlock_release_readlock(rwlock_t *rw);
void rwlock_acquire_writelock(rwlock_t *rw);
void rwlock_release_writelock(rwlock_t *rw);
void *reader(void *arg);
void *writer(void *arg);
double get_elapsed_time(struct timeval *start_time);

// 전역 변수
struct timeval start_time;

/**
 * @brief 현재 시간과 프로그램 시작 시간의 차이를 초 단위로 계산
 * @param start_time 프로그램 시작 시간
 * @return 경과 시간 (초)
 */
double get_elapsed_time(struct timeval *start_time) {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    double elapsed = (current_time.tv_sec - start_time->tv_sec) + 
                     (current_time.tv_usec - start_time->tv_usec) / 1000000.0;
    return elapsed;
}

// Reader-Writer Lock 초기화 함수
void rwlock_init(rwlock_t *rw) {
    rw->readers = 0;
    rw->writers = 0;
    sem_init(&rw->mutex, 0, 1);
    sem_init(&rw->writelock, 0, 1);
    sem_init(&rw->rlock, 0, 1);
}

// 독자 스레드가 락을 획득하는 함수
void rwlock_acquire_readlock(rwlock_t *rw) {
    // 저자가 진입을 시도하면 rlock이 잠기므로, 새로운 독자는 여기서 대기
    sem_wait(&rw->rlock);
    sem_post(&rw->rlock);

    sem_wait(&rw->mutex);
    rw->readers++;
    // 첫 번째 독자라면, 저자의 진입을 막기 위해 writelock을 잠금
    if (rw->readers == 1) {
        sem_wait(&rw->writelock);
    }
    sem_post(&rw->mutex);
}

// 독자 스레드가 락을 해제하는 함수
void rwlock_release_readlock(rwlock_t *rw) {
    sem_wait(&rw->mutex);
    rw->readers--;
    // 마지막 독자라면, 대기 중인 저자를 위해 writelock을 풀어줌
    if (rw->readers == 0) {
        sem_post(&rw->writelock);
    }
    sem_post(&rw->mutex);
}

// 저자 스레드가 락을 획득하는 함수
void rwlock_acquire_writelock(rwlock_t *rw) {
    sem_wait(&rw->mutex);
    rw->writers++;
    // 첫 번째 저자라면, 새로운 독자들의 진입을 막기 위해 rlock을 잠금
    if (rw->writers == 1) {
        sem_wait(&rw->rlock);
    }
    sem_post(&rw->mutex);
    
    // 배타적 접근을 위해 writelock을 잠금
    sem_wait(&rw->writelock);
}

// 저자 스레드가 락을 해제하는 함수
void rwlock_release_writelock(rwlock_t *rw) {
    sem_post(&rw->writelock);

    sem_wait(&rw->mutex);
    rw->writers--;
    // 대기 중인 저자가 없다면, 독자들의 진입을 허용
    if (rw->writers == 0) {
        sem_post(&rw->rlock);
    }
    sem_post(&rw->mutex);
}

// 독자 스레드 함수
void *reader(void *arg) {
    thread_arg_t *t_arg = (thread_arg_t *)arg;
    
    printf("[%0.4f] Reader#%d: Created!\n", get_elapsed_time(t_arg->start_time), t_arg->id);
    
    rwlock_acquire_readlock(t_arg->rw);
    
    printf("[%0.4f] Reader#%d: Read started! (reading %d ms)\n", get_elapsed_time(t_arg->start_time), t_arg->id, t_arg->process_time);
    usleep(t_arg->process_time * 1000); // ms를 us로 변환하여 sleep
    
    printf("[%0.4f] Reader#%d: Terminated\n", get_elapsed_time(t_arg->start_time), t_arg->id);
    
    rwlock_release_readlock(t_arg->rw);
    
    free(t_arg);
    return NULL;
}

// 저자 스레드 함수
void *writer(void *arg) {
    thread_arg_t *t_arg = (thread_arg_t *)arg;

    printf("[%0.4f] Writer#%d: Created!\n", get_elapsed_time(t_arg->start_time), t_arg->id);
    
    rwlock_acquire_writelock(t_arg->rw);
    
    printf("[%0.4f] Writer#%d: Write started! (writing %d ms)\n", get_elapsed_time(t_arg->start_time), t_arg->id, t_arg->process_time);
    usleep(t_arg->process_time * 1000); // ms를 us로 변환하여 sleep
    
    printf("[%0.4f] Writer#%d: Terminated\n", get_elapsed_time(t_arg->start_time), t_arg->id);

    rwlock_release_writelock(t_arg->rw);

    free(t_arg);
    return NULL;
}

// 메인 함수
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./rwlock <sequence file>\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("Failed to open sequence file");
        return 1;
    }

    gettimeofday(&start_time, NULL);

    rwlock_t rw;
    rwlock_init(&rw);

    pthread_t threads[100]; // 최대 100개의 스레드 지원
    int thread_count = 0;
    int reader_id = 0;
    int writer_id = 0;

    char type;
    int process_time;

    // sequence 파일에서 한 줄씩 읽어 스레드 생성
    while (fscanf(fp, " %c %d", &type, &process_time) == 2) {
        thread_arg_t *arg = (thread_arg_t *)malloc(sizeof(thread_arg_t));
        if (arg == NULL) {
            perror("Failed to allocate memory for thread argument");
            continue;
        }
        arg->type = type;
        arg->process_time = process_time;
        arg->rw = &rw;
        arg->start_time = &start_time;

        if (type == 'R') { // 독자 스레드 생성
            arg->id = ++reader_id;
            pthread_create(&threads[thread_count++], NULL, reader, arg);
        } else if (type == 'W') { // 저자 스레드 생성
            arg->id = ++writer_id;
            pthread_create(&threads[thread_count++], NULL, writer, arg);
        } else {
            fprintf(stderr, "Invalid thread type: %c\n", type);
            free(arg);
        }
        
        usleep(100000); // 100ms 대기
    }

    // 모든 스레드가 종료될 때까지 대기
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("End of sequence.\n");

    fclose(fp);
    return 0;
}