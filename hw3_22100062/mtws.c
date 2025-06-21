#define _GNU_SOURCE // strcasestr() 사용을 위해 필요
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/stat.h>

// 스레드간 공유될 데이터 구조체
typedef struct {
    char** buffer;
    int capacity;
    int count;
    int in;
    int out;
    pthread_mutex_t mutex; 
    pthread_cond_t cond_full;
    pthread_cond_t cond_empty;
} BoundedBuffer;

// 전역 변수
BoundedBuffer bounded_buffer;
char* search_word;
int producer_done = 0;

long total_word_count = 0;
int total_file_count = 0;
pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;

// 함수
void initialize_buffer(int size);
void destroy_buffer();
void put_item(const char* path);
char* get_item();
void list_files_recursive(const char* base_path);
void* producer_thread_func(void* arg);
void* consumer_thread_func(void* arg);

void initialize_buffer(int size) {
    bounded_buffer.capacity = size;
    bounded_buffer.buffer = (char**)malloc(sizeof(char*) * size);
    bounded_buffer.count = 0;
    bounded_buffer.in = 0;
    bounded_buffer.out = 0;
    pthread_mutex_init(&bounded_buffer.mutex, NULL);
    pthread_cond_init(&bounded_buffer.cond_full, NULL);
    pthread_cond_init(&bounded_buffer.cond_empty, NULL);
}

void destroy_buffer() {
    free(bounded_buffer.buffer);
    pthread_mutex_destroy(&bounded_buffer.mutex);
    pthread_cond_destroy(&bounded_buffer.cond_full);
    pthread_cond_destroy(&bounded_buffer.cond_empty);
    pthread_mutex_destroy(&result_mutex);
}

void put_item(const char* path) {
    pthread_mutex_lock(&bounded_buffer.mutex);
    while (bounded_buffer.count == bounded_buffer.capacity) {
        pthread_cond_wait(&bounded_buffer.cond_full, &bounded_buffer.mutex);
    }

    bounded_buffer.buffer[bounded_buffer.in] = strdup(path); // 경로 복사
    bounded_buffer.in = (bounded_buffer.in + 1) % bounded_buffer.capacity;
    bounded_buffer.count++;

    pthread_cond_signal(&bounded_buffer.cond_empty);
    pthread_mutex_unlock(&bounded_buffer.mutex);
}

char* get_item() {
    pthread_mutex_lock(&bounded_buffer.mutex);
    while (bounded_buffer.count == 0 && !producer_done) {
        pthread_cond_wait(&bounded_buffer.cond_empty, &bounded_buffer.mutex);
    }

    if (bounded_buffer.count == 0 && producer_done) {
        pthread_mutex_unlock(&bounded_buffer.mutex);
        return NULL;
    }

    char* path = bounded_buffer.buffer[bounded_buffer.out];
    bounded_buffer.out = (bounded_buffer.out + 1) % bounded_buffer.capacity;
    bounded_buffer.count--;

    // 버퍼에 빈 공간이 생겼음
    pthread_cond_signal(&bounded_buffer.cond_full);
    pthread_mutex_unlock(&bounded_buffer.mutex);
    return path;
}

void list_files_recursive(const char* base_path) {
    DIR* dir = opendir(base_path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);

        struct stat statbuf;
        if (stat(path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                list_files_recursive(path);
            } else if (S_ISREG(statbuf.st_mode)) {
                put_item(path);
            }
        }
    }
    closedir(dir);
}

void* producer_thread_func(void* arg) {
    char* search_dir = (char*)arg;
    list_files_recursive(search_dir);

    // 모든 파일 탐색 완료
    pthread_mutex_lock(&bounded_buffer.mutex);
    producer_done = 1;
    pthread_cond_broadcast(&bounded_buffer.cond_empty);
    pthread_mutex_unlock(&bounded_buffer.mutex);
    
    return NULL;
}

void* consumer_thread_func(void* arg) {
    long thread_id = (long)arg;
    
    printf("[Thread#%ld] started searching '%s'...\n", thread_id, search_word);

    while (1) {
        char* file_path = get_item();
        if (file_path == NULL) {
            // 처리할 파일이 더 이상 없으면 스레드 종료
            break;
        }

        FILE* fp = fopen(file_path, "r");
        if (!fp) {
            // perror(file_path);
            free(file_path);
            continue;
        }

        int count = 0;
        char* line = NULL;
        size_t len = 0;
        // 파일의 각 줄을 읽으며 단어 검색
        while (getline(&line, &len, fp) != -1) {
            char* pos = line;
            // strcasestr은 대소문자를 구분하지 않고 문자열을 검색
            while ((pos = strcasestr(pos, search_word)) != NULL) {
                count++;
                pos += strlen(search_word);
            }
        }
        fclose(fp);
        if(line) free(line);

        // mutex
        pthread_mutex_lock(&result_mutex);
        int current_file_index = total_file_count++;
        total_word_count += count;
        pthread_mutex_unlock(&result_mutex);

        printf("[Thread#%ld-%d] %s: %d found\n", thread_id, current_file_index, file_path, count);
        free(file_path);
    }

    return NULL;
}

void print_usage() {
    printf("Usage: ./multi_t\n");
    printf("  -b: bounded buffer size\n");
    printf("  -t: number of threads searching word (except for main thread)\n");
    printf("  -d: search directory\n");
    printf("  -w: search word\n");
}

int main(int argc, char* argv[]) {
    int opt;
    int buffer_size = -1, num_threads = -1;
    char* search_dir = NULL;

    while ((opt = getopt(argc, argv, "b:t:d:w:")) != -1) {
        switch (opt) {
            case 'b':
                buffer_size = atoi(optarg);
                break;
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'd':
                search_dir = optarg;
                break;
            case 'w':
                search_word = optarg;
                break;
            default:
                print_usage();
                return 1;
        }
    }

    // check if required parameters are provided
    if (buffer_size <= 0 || num_threads <= 0 || search_dir == NULL || search_word == NULL) {
        print_usage();
        return 1;
    }

    printf("Buffer size=%d, Num threads=%d, Directory=%s, SearchWord=%s\n", 
           buffer_size, num_threads, search_dir, search_word);

    
    initialize_buffer(buffer_size);

    // producer
    pthread_t producer_tid;
    pthread_create(&producer_tid, NULL, producer_thread_func, (void*)search_dir);

    // consumer
    pthread_t* consumer_tids = malloc(sizeof(pthread_t) * num_threads);
    for (long i = 0; i < num_threads; i++) {
        pthread_create(&consumer_tids[i], NULL, consumer_thread_func, (void*)i);
    }

    // 모든 스레드가 종료될 때까지
    pthread_join(producer_tid, NULL);
    for (int i = 0; i < num_threads; i++) {
        pthread_join(consumer_tids[i], NULL);
    }

    printf("Total found = %ld (Num files=%d)\n", total_word_count, total_file_count);

    free(consumer_tids);
    destroy_buffer();

    return 0;
}