#include "logging.h"
#include "logging.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <sys/file.h>

#define BUFFER_SIZE 128
#define BUFF_S 296
#define ARGS_NUM 4
int register_pipe;
char *pipe_name;
char *message_box;

void send_msg(int tx, char const *str) {
    size_t len = strlen(str);
    size_t written = 0;

    while (written < len) {
        ssize_t ret = write(tx, str + written, len - written);
        if (ret < 0) {
            fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        written += (size_t)ret;
    }
}

void sig_handler(int sig){
    printf("\nCaught signal %d\n", sig);
    
    if (close(register_pipe) == -1)
    {
        printf("Erro no close\n");
    }

    exit(1);
}

int main(int argc, char **argv) {
    signal(SIGINT, sig_handler);
    if(argc != ARGS_NUM){
        printf("Numero de argumento nao esta correto.\n");
        return -1;
    }
    char *user_name = argv[0];
    user_name += 2;

    char *register_pipe_name = argv[1];
    pipe_name = argv[2];
    message_box = argv[3];
    
    

    register_pipe = open(register_pipe_name, O_WRONLY);
    if(register_pipe == -1){
        sig_handler(-1);
    }
    char buffer[BUFF_S];
    flock(register_pipe, LOCK_EX);
    strcat(buffer, "2|");
    strcat(buffer, pipe_name);
    strcat(buffer, "|");
    strcat(buffer, message_box);
    send_msg(register_pipe, buffer);
    flock(register_pipe, LOCK_UN);
    close(register_pipe);
    
    return -1;
}