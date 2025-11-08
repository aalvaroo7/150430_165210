#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include "ops.h"

#define BACKLOG 64
#define BUFSZ 256

typedef struct { int fd; } worker_arg;

static void *worker(void *argp){
    worker_arg *arg = (worker_arg*)argp;
    int fd = arg->fd; free(arg);
    FILE *io = fdopen(fd, "r+");
    if (!io){ close(fd); return NULL; }
    char line[BUFSZ];
    while (fgets(line, sizeof(line), io)){
        request_t req; char em[64]={0}; int64_t out=0;
        if (!parse_request(line, &req, em, sizeof(em))){
            fprintf(io, "ERR %s\n", em); fflush(io); continue;
        }
        if (!compute(&req, &out, em, sizeof(em))){
            fprintf(io, "ERR %s\n", em); fflush(io); continue;
        }
        fprintf(io, "OK %lld\n", (long long)out); fflush(io);
    }
    fclose(io);
    return NULL;
}

int main(int argc, char **argv){
    int port = (argc>1)? atoi(argv[1]) : 5001;
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s<0){ perror("socket"); return 1; }
    int yes=1; setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr; bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET; addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(port);
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr))<0){ perror("bind"); return 1; }
    if (listen(s, BACKLOG)<0){ perror("listen"); return 1; }

    for(;;){
        struct sockaddr_in cli; socklen_t clilen=sizeof(cli);
        int c = accept(s, (struct sockaddr*)&cli, &clilen);
        if (c<0){ perror("accept"); continue; }
        pthread_t th; pthread_attr_t at; pthread_attr_init(&at);
        pthread_attr_setdetachstate(&at, PTHREAD_CREATE_DETACHED);
        worker_arg *wa = malloc(sizeof(worker_arg)); wa->fd = c;
        if (pthread_create(&th, &at, worker, wa)!=0){ perror("pthread_create"); close(c); free(wa); }
        pthread_attr_destroy(&at);
    }
}

