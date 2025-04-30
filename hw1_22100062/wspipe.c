#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1
#define BUFFER_SIZE 1024

ssize_t read_line(int fd, char *buffer, size_t max_length);

char* my_strcasestr(const char *str, const char *to_find) {
    int i, j;

    if (to_find[0] == '\0') {
        return (char *)str;
    }

    for (i = 0; str[i] != '\0'; i++) {
        for (j = 0; to_find[j] != '\0'; j++) {
            if (tolower((unsigned char)str[i + j]) != tolower((unsigned char)to_find[j])) {
                break;
            }
        }
        if (to_find[j] == '\0') {
            return (char *)(str + i);
        }
    }
    return NULL;
}

char* my_strstr(const char *str, const char *to_find) {
    int i, j;

    if (to_find[0] == '\0') {
        return (char *)str;
    }

    for (i = 0; str[i] != '\0'; i++) {
        for (j = 0; to_find[j] != '\0'; j++) {
            if (str[i + j] != to_find[j]) {
                break;
            }
        }
        if (to_find[j] == '\0') {
            return (char *)(str + i);
        }
    }
    return NULL;
}


int main(int argc, char *argv[])
{
    pid_t pid;
    int fd[2];
    int i;
    char buffer[BUFFER_SIZE];
    char *command, *word;
    int total_count = 0;
    int grep_mode = 0;

    if (argc < 3)
    {
        printf("Usage: %s [-g] <command> <word>\n", argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "-g") == 0)
    {
        grep_mode = 1;
        command = argv[2];
        word = argv[3];
    }
    else
    {
        command = argv[1];
        word = argv[2];
    }

    if (pipe(fd) == -1)
    {
        fprintf(stderr, "Pipe failed\n");
        return 1;
    }

    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork failed\n");
        return 1;
    }

    if (pid > 0)
    {
        close(fd[WRITE_END]);
        i = 1;
        char *ptr, *cur;

        while (read_line(fd[READ_END], buffer, BUFFER_SIZE - 1) != 0)
        {
            cur = buffer;
            int found = (my_strstr(cur, word) != NULL);

            if (!grep_mode || (grep_mode && found))
            {
                printf("%d: ", i);

                while ((ptr = my_strstr(cur, word)) != NULL)
                {
                    printf("%.*s", (int)(ptr - cur), cur);
                    printf("\033[31m%s\033[0m", word);
                    cur = ptr + strlen(word);
                }
                printf("%s", cur);

                if (found)
                    total_count++;
            }
            i++;
        }
        close(fd[READ_END]);
        wait(NULL);
        printf("\nTotal occurrences of '%s': %d\n", word, total_count);
    }
    else
    {
        // Child process
        close(fd[READ_END]);
        dup2(fd[WRITE_END], STDOUT_FILENO);
        close(fd[WRITE_END]);
        execl("/bin/sh", "sh", "-c", command, NULL);
        exit(1);
    }

    return 0;
}

ssize_t read_line(int fd, char *buffer, size_t max_length)
{
    size_t num_read = 0;
    char c;
    while (read(fd, &c, 1) == 1 && num_read < max_length)
    {
        buffer[num_read++] = c;
        if (c == '\n')
        {
            break;
        }
    }
    buffer[num_read] = '\0';
    return num_read;
}
