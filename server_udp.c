#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include "ops.h"

#define BUFSZ 512

int main(int argc, char **argv){
    int port = (argc>1)? atoi(argv[1]) : 5002;
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s<0){ perror("socket"); return 1; }
    struct sockaddr_in addr; bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET; addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(port);
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr))<0){ perror("bind"); return 1; }

    for(;;){
        char buf[BUFSZ]; struct sockaddr_in cli; socklen_t clen=sizeof(cli);
        ssize_t n = recvfrom(s, buf, sizeof(buf)-1, 0,(struct sockaddr*)&cli,&clen);
        if (n<0){ perror("recvfrom"); continue; }
        buf[n]='\0';
        request_t req; char em[64]={0}; int64_t out=0;
        char outbuf[128];
        if (!parse_request(buf, &req, em, sizeof(em))){
            snprintf(outbuf, sizeof(outbuf), "ERR %s\n", em);
        } else if (!compute(&req, &out, em, sizeof(em))){
            snprintf(outbuf, sizeof(outbuf), "ERR %s\n", em);
        } else {
            snprintf(outbuf, sizeof(outbuf), "OK %lld\n", (long long)out);
        }
        sendto(s, outbuf, strlen(outbuf), 0, (struct sockaddr*)&cli, clen);
    }
}
