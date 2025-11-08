#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv){
    if (argc<3){ fprintf(stderr,"uso: %s <host> <puerto>\n", argv[0]); return 1; }
    const char *host=argv[1]; int port=atoi(argv[2]);

    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s<0){ perror("socket"); return 1; }

    struct sockaddr_in addr; memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET; addr.sin_port=htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr)<=0){
        struct hostent *he = gethostbyname(host);
        if (!he){ fprintf(stderr,"host\n"); return 1; }
        memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    }
    if (connect(s,(struct sockaddr*)&addr,sizeof(addr))<0){ perror("connect"); return 1; }

    FILE *io = fdopen(s, "r+"); if (!io){ perror("fdopen"); close(s); return 1; }
    char line[256];
    while (fgets(line, sizeof(line), stdin)){
        fputs(line, io); fflush(io);
        if (!fgets(line, sizeof(line), io)) break;
        fputs(line, stdout);
    }
    fclose(io);
    return 0;
}
