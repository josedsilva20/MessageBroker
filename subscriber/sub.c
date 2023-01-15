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

#define PIPE_SIZE 256
#define BOX_SIZE 32
#define BUFF_S 296
#define ARGS_NUM 4
#define PIPE_NAME 2
#define MSG_BOX 3

int register_pipe;
char pipe_name[PIPE_SIZE];

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

void transform_name_pipe(){
    size_t j = strlen(pipe_name);
    for(;j < PIPE_SIZE; j++){
        pipe_name[j] = '\0';
    } 
}

int main(int argc, char **argv) {
    signal(SIGINT, sig_handler);
    if(argc != ARGS_NUM){
        printf("Numero de argumento nao esta correto.\n");
        return -1;
    }
    char *register_pipe_name = argv[1];
    strcpy(pipe_name,argv[PIPE_NAME]);
    char *message_box = argv[MSG_BOX];
    

    register_pipe = open(register_pipe_name, O_WRONLY);
    if(register_pipe == -1){
        sig_handler(-1);
    }
    char buffer[296];
    flock(register_pipe, LOCK_EX);
    strcat(buffer, "1|");
    strcat(buffer, pipe_name);
    strcat(buffer, "|");
    strcat(buffer, message_box);
    strcat(buffer, "");
    send_msg(register_pipe, buffer);
    flock(register_pipe, LOCK_UN);
    close(register_pipe);
    unlink(pipe_name);
    
    return -1;
}