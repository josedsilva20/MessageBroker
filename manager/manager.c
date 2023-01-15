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
int register_pipe;
char *pipe_name;

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
int mbroker_create(char *named_pipe){
    unlink(named_pipe);
    // Create pipe
    if (mkfifo(named_pipe, 0640) != 0) {
        fprintf(stderr, "[ERR]: mkfifo failed:\n");
        return -1;
    }
    return 0;
}

char* transform_pipe_name(char *named_pipe){
    char *final = "";
    char *nada = "\0";
    for(int i = 0; i < 256; i++){
        if(i < strlen(named_pipe)){
            final[i] = named_pipe[i];
        }
        else{
            final[i] = nada[0]; 
        }
    }
    return final;
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
    if(argc < 4){
        printf("Numero de argumento nao esta correto.\n");
        return -1;
    }
    char *user_name = argv[0];
    user_name += 2;

    char *register_pipe_name = argv[1];
    pipe_name = argv[2];
    char *func = argv[3];
    char *box_name;
    if(argc == 5){
        box_name = argv[4];
    }
    

    register_pipe = open(register_pipe_name, O_WRONLY);
    if(register_pipe == -1){
        sig_handler(-1);
    }
    char buffer[BUFF_S];
    flock(register_pipe, LOCK_EX);
    if(strcmp(func, "create") == 0){
        strcat(buffer, "3|");
        strcat(buffer, pipe_name);
        strcat(buffer, "|");
        strcat(buffer, box_name);
        
    }
    else if(strcmp(func, "remove") == 0){
        strcat(buffer, "5|");
        strcat(buffer, pipe_name);
        strcat(buffer, "|");
        strcat(buffer, box_name);
        
    }
    else if(strcmp(func, "list") == 0){
        strcat(buffer, "7|");
        strcat(buffer, pipe_name);
    }
    else{
        printf("Comando desconhecido\n");
        sig_handler(-1);
    }
    send_msg(register_pipe, buffer);
    flock(register_pipe, LOCK_UN);
    close(register_pipe);
    sleep(1);
    char buf[BUFFER_SIZE];
        printf("%s\n", pipe_name);
        int pipe_read = open(pipe_name, O_RDONLY);
        if (pipe_read == -1)
        {
            printf("Erro abrir pipe\n");
            sig_handler(-1);
        }
        while (1)
        {   
            
            ssize_t bytes_read_name = read(pipe_read, buf, BUFFER_SIZE - 1);
            
            if(bytes_read_name == -1){
                printf("Erro read\n");
                sig_handler(-1);
            }
            if(bytes_read_name == 0){
                break;
            }
            
            buf[bytes_read_name] = 0;
            printf("%s\n", buf);
            
            
        }
    return -1;
}