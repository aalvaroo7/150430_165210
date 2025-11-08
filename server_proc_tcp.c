#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "ops.h"

#define BACKLOG 16
#define BUFSZ 256

static void reap(int sig){ (void)sig; while (waitpid(-1,NULL,WNOHANG)>0){} }

static void handle_client(int fd){
    FILE *io = fdopen(fd, "r+");
    if (!io) { close(fd); return; }
    char line[BUFSZ];
    while (fgets(line, sizeof(line), io)){
        request_t req; char em[64]={0}; int64_t out=0;
        if (!parse_request(line, &req, em, sizeof(em))){
            fprintf(io, "ERR %s\n", em); fflush(io);
            continue;
        }
        if (!compute(&req, &out, em, sizeof(em))){
            fprintf(io, "ERR %s\n", em); fflush(io);
            continue;
        }
        fprintf(io, "OK %lld\n", (long long)out); fflush(io);
    }
    fclose(io);
}

int main(int argc, char **argv){
    int port = (argc>1)? atoi(argv[1]) : 5000;
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s<0){ perror("socket"); return 1; }
    int yes=1; setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr; bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET; addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(port);
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr))<0){ perror("bind"); return 1; }
    if (listen(s, BACKLOG)<0){ perror("listen"); return 1; }

    struct sigaction sa={0}; sa.sa_handler=reap; sigemptyset(&sa.sa_mask);
    sa.sa_flags=SA_RESTART; sigaction(SIGCHLD,&sa,NULL);

    for(;;){
        struct sockaddr_in cli; socklen_t clilen=sizeof(cli);
        int c = accept(s, (struct sockaddr*)&cli, &clilen);
        if (c<0){ if (errno==EINTR) continue; perror("accept"); continue; }
        pid_t pid=fork();
        if (pid==0){ close(s); handle_client(c); _exit(0); }
        close(c);
    }
}

