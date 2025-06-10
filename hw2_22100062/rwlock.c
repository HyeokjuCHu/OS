/*
기본적인 해결방법 설명
Reader 또는 Writer가 대기 중임을 나타내는
추가적인 카운터나 세마포어를 사용하고, lock 획득 결정 시 이 정보를 활용

아이디어:https://ui.adsabs.harvard.edu/abs/2003cs........3005B/abstract
"The known solutions to 
this problem typically involve a number of global counters and queues"
여기서 counter를 사용하기로 결정.
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

typedef struct _rwlock_t {
    sem_t writelock;
    sem_t mutex;
    sem_t fairlock;
    int readers;
} rwlock_t;

// Structure for thread arguments
typedef struct _thread_arg_t {
    int id;
    char type;
    int process_time;
    rwlock_t *rw;
    struct timeval *start_time;
} thread_arg_t;

// Function prototypes
void rwlock_init(rwlock_t *rw);
void rwlock_acquire_readlock(rwlock_t *rw);
void rwlock_release_readlock(rwlock_t *rw);
void rwlock_acquire_writelock(rwlock_t *rw);
void rwlock_release_writelock(rwlock_t *rw);
void *reader(void *arg);
void *writer(void *arg);
double get_elapsed_time(struct timeval *start_time);

// Global variable for program start time
struct timeval start_time;

double get_elapsed_time(struct timeval *start_time) {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    double elapsed = (current_time.tv_sec - start_time->tv_sec) + 
                     (current_time.tv_usec - start_time->tv_usec) / 1000000.0;
    return elapsed;
}

// Initializes the Reader-Writer Lock
void rwlock_init(rwlock_t *rw) {
    rw->readers = 0;
    sem_init(&rw->mutex, 0, 1);
    sem_init(&rw->writelock, 0, 1);
    sem_init(&rw->fairlock, 0, 1);
}

// Allows a reader thread to acquire the lock
void rwlock_acquire_readlock(rwlock_t *rw) {

    sem_wait(&rw->fairlock);
    sem_post(&rw->fairlock); // Release immediately so other reader threads can enter
    
    sem_wait(&rw->mutex);
    rw->readers++;
    if (rw->readers == 1) {
        sem_wait(&rw->writelock);
    }
    sem_post(&rw->mutex);
}

// Allows a reader thread to release the lock
void rwlock_release_readlock(rwlock_t *rw) {
    sem_wait(&rw->mutex);
    rw->readers--;
    if (rw->readers == 0) {
        sem_post(&rw->writelock);
    }
    sem_post(&rw->mutex);
}

// Allows a writer thread to acquire the lock
void rwlock_acquire_writelock(rwlock_t *rw) {
    sem_wait(&rw->fairlock);
    
    sem_wait(&rw->writelock);
}

// Allows a writer thread to release the lock
void rwlock_release_writelock(rwlock_t *rw) {
    sem_post(&rw->writelock);
    sem_post(&rw->fairlock);
}

// Function for reader threads
void *reader(void *arg) {
    thread_arg_t *t_arg = (thread_arg_t *)arg;
    
    printf("[%0.4f] Reader#%d: Created!\n", get_elapsed_time(t_arg->start_time), t_arg->id);
    
    rwlock_acquire_readlock(t_arg->rw);
    
    printf("[%0.4f] Reader#%d: Read started! (reading %d ms)\n", get_elapsed_time(t_arg->start_time), t_arg->id, t_arg->process_time);
    usleep(t_arg->process_time * 1000); // Convert ms to us for sleep
    
    printf("[%0.4f] Reader#%d: Terminated\n", get_elapsed_time(t_arg->start_time), t_arg->id);
    
    rwlock_release_readlock(t_arg->rw);
    
    free(t_arg);
    return NULL;
}

// Function for writer threads
void *writer(void *arg) {
    thread_arg_t *t_arg = (thread_arg_t *)arg;

    printf("[%0.4f] Writer#%d: Created!\n", get_elapsed_time(t_arg->start_time), t_arg->id);
    
    rwlock_acquire_writelock(t_arg->rw);
    
    printf("[%0.4f] Writer#%d: Write started! (writing %d ms)\n", get_elapsed_time(t_arg->start_time), t_arg->id, t_arg->process_time);
    usleep(t_arg->process_time * 1000); // Convert ms to us for sleep
    
    printf("[%0.4f] Writer#%d: Terminated\n", get_elapsed_time(t_arg->start_time), t_arg->id);

    rwlock_release_writelock(t_arg->rw);

    free(t_arg);
    return NULL;
}

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

    pthread_t threads[100]; // Supports up to 100 threads 
    int thread_count = 0;
    int reader_id = 0;
    int writer_id = 0;

    char type;
    int process_time;

    // Read lines from the sequence file and create threads
    while (fscanf(fp, " %c %d", &type, &process_time) == 2) { // 
        thread_arg_t *arg = (thread_arg_t *)malloc(sizeof(thread_arg_t));
        if (arg == NULL) {
            perror("Failed to allocate memory for thread argument");
            continue;
        }
        arg->type = type;
        arg->process_time = process_time; // 
        arg->rw = &rw;
        arg->start_time = &start_time;

        if (type == 'R') { // Create a reader thread 
            arg->id = ++reader_id;
            pthread_create(&threads[thread_count++], NULL, reader, arg);
        } else if (type == 'W') { // Create a writer thread 
            arg->id = ++writer_id;
            pthread_create(&threads[thread_count++], NULL, writer, arg);
        } else {
            fprintf(stderr, "Invalid thread type: %c\n", type);
            free(arg);
        }
        
        usleep(100000); // Wait for 100ms 
    }

    // Wait for all threads to finish
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("End of sequence.\n");

    fclose(fp);
    return 0;
}