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
#define BUFF_S 256
#define NAME_SIZE 32

char *pipe_name;

struct Subscriber_pipe{
    char subscriber_pipe[BUFF_S];
    struct Subscriber_pipe* next;
}sub_t;

struct Box{
    char name[NAME_SIZE];
    int box_file;
    char publisher_pipe[BUFF_S];
    struct Subscriber_pipe *subscriber_pipe;
    struct Box *next;
}box_t;



struct Box *head = NULL;

void send_msg(char *name, char const *str) {
    printf("%s\n", name);
    int pp = open(name, O_WRONLY);
    if(pp == -1){
        printf("open error\n");
    }
    size_t len = strlen(str);
    size_t written = 0;

    while (written < len) {
        ssize_t ret = write(pp, str + written, len - written);
        if (ret < 0) {
            fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        written += (size_t)ret;
    }
    close(pp);
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

int box_create(char *name){
    struct Box *new = malloc(sizeof(struct Box));
    strcpy(new->name, name);
    new->subscriber_pipe =NULL;
    new->box_file = creat(name, O_CREAT);
    new->next = NULL;
    if(head == NULL){
        head = new;
        printf("creacao %s\n", head->name);
        if(head == NULL){
            return -1;
        }
        return 0;
    }

    else{
        struct Box *prox = head;
        while(1){
            if(prox->next == NULL){
                break;
            }
            else{
                prox = prox->next;
            }
        }
        prox->next = new;
        if(prox->next == NULL){
            return -1;
        }
        return 0;
    }
}

int box_remove(char *name){
    struct Box *box = head;
    printf("%s\n", box->name);
    if(strcmp(head->name, name) == 0){
        printf("%s  %s\n", box->name, name);
        head = head->next;
        
        free(box);
        return 0;
    }
    
    struct Box *prox = box->next;
    while(box != NULL){
        if(strcmp(prox->name, name) == 0){
            box->next = prox->next;
            free(prox);
            
            return 0;
        }
        box = prox;
        prox = box->next;
    }
    return -1;
}

int box_exist(char *name){
    struct Box *box = head;
    if(box == NULL){
        return -1;
    }
    while(box != NULL){
        if(strcmp(box->name, name) == 0){
            return 1;
        }
        box = box->next;
    }
    return -1;
}


struct Box *box_search(char *name){
    struct Box *box = head;
    if(box == NULL){
        return NULL;
    }
    if(strcmp(box->name, name) == 0){
        return box;
    }
    while(box->next != NULL){
        if(strcmp(box->name, name) == 0){
            return box;
        }
        box = box->next;
    }
    return NULL;
}

int box_without_publisher(char *name){
    struct Box *box;
    box = box_search(name);
    if(box == NULL){
        return -1;
    }
    if(strlen(box->publisher_pipe) == 0){
        return 1;
    }
    printf("%s", box->publisher_pipe);
    return -1;
}

void publisher_to_box(char *box_name, char* publisher_pipe){
    struct Box *box;
    box = box_search(box_name);
    if(box == NULL){
        return;
    }
    strcpy(box->publisher_pipe, publisher_pipe);
}

void subscriber_to_box(char *name, char *sub_pipe){

    struct Subscriber_pipe *new = malloc(sizeof(struct Subscriber_pipe));
    new->next =NULL;
    strcpy(new->subscriber_pipe, sub_pipe);
    struct Box *box;
    box = box_search(name);
    if(box != NULL){
        struct Subscriber_pipe *prox = box->subscriber_pipe;
    if(prox == NULL){
        box->subscriber_pipe = new;
        return;
    }else{
    while(prox->next != NULL){
        prox = prox->next;
    }
    prox->next = new;
    }}
}

int loggin(char *str){
    int code;
    char buffer[strlen(str) - 1];
    char *client_pipe;
    char *box_name;
    sscanf(str,"%d|%s", &code, buffer);
    switch (code)
    {
    case 1: 
        client_pipe = strtok(buffer, "|");
        box_name = strtok(NULL, "|");
        mbroker_create(client_pipe);
        if(box_exist(box_name) == 1){
            subscriber_to_box(box_name, client_pipe);
        }
        else{
            printf("box n existe");
        }
        printf("Subscriber\n");
        break;
    case 2: 
        client_pipe = strtok(buffer, "|");
        box_name = strtok(NULL, "|");
        mbroker_create(client_pipe);
        if(box_exist(box_name) == 1){
            
            if(box_without_publisher(box_name) == 1){
                publisher_to_box(box_name, client_pipe);
                printf("Publisher login sucess\n");
            }
            else{
                printf("Box ja tem publisher\n");
            }
        }
        else{
            printf("Box n existe\n");
        }
        
        break;
    case 3: 
        client_pipe = strtok(buffer, "|");
        box_name = strtok(NULL, "|");
        mbroker_create(client_pipe);
        
        if(box_exist(box_name) == -1){
            box_create(box_name);
            char *ret = "4|0|";
            send_msg(client_pipe, ret);
            }
            else{
                char *ret = "4|-1|ja existe";
                send_msg(client_pipe, ret);
            }
        printf("Create\n");
        break;
    case 5: 
        client_pipe = strtok(buffer, "|"); 
        box_name = strtok(NULL, "|");
        mbroker_create(client_pipe);
        if(box_exist(box_name) != 1){
            box_remove(box_name);
        }
        else{
            printf("Box nao existe\n");
            return -1;
            }
        printf("Remove\n");
        break;
    case 7: 
        (void)client_pipe;
        (void)box_name;
        printf("List\n");
        break;
    default:
        (void)client_pipe;
        (void)box_name;
        break;
    }
    return 0;
}






void sig_handler(int sig){
    unlink(pipe_name);
    printf("\nCaught signal %d\n", sig);
    //sem_unlink(SEM_NAME);
    exit(0);
}




int main(int argc, char **argv) {

    if(argc != 3){
        printf("Numero de argumento nao esta correto.\n");
        return -1;
    }
    pipe_name = argv[1];
    int  max_session;
    sscanf(argv[2], "%d", &max_session);
    int sessions = 0;
    

    signal(SIGINT, sig_handler);
    char buf[BUFFER_SIZE];
    if(mbroker_create(pipe_name) != 0){
        printf("named pipe created not sussed");
    }
    while(sessions < max_session){
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
                flock(pipe_read, LOCK_UN);
                break;
            }
            
            buf[bytes_read_name] = 0;
            printf("%s\n", buf);
            loggin(buf);
            
            
        }
        sessions += 1;
    }
    return -1;
}