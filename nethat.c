#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

int verbose = 0;
int slots = 32;
char *port = NULL;
int family = AF_UNSPEC;

int server = 0;
int *clients = NULL;

static struct option options[] = {
    {"verbose", no_argument, 0, 'v'},
    {"port", required_argument, 0, 'p'},
    {"ipv4", required_argument, 0, '4'},
    {"ipv6", required_argument, 0, '6'},
    {"slots", required_argument, 0, 'n'},
    {0, 0, 0, 0}
};

int parse_args(int argc, char **argv){
    char c;
    while((c = getopt_long(argc, argv, "vp:n:46", options, NULL)) != -1){
        switch(c){
            case 'v':
                verbose = 1;
                break;
            case 'p':
                port = optarg;
                break;
            case 'n':
                slots = atoi(optarg);
                if(slots <= 0){
                    fprintf(stderr, "invalid number of slots: %s\n", optarg);
                    return 0;
                }
                break;
            case '4':
                family = AF_INET;
                break;
            case '6':
                family = AF_INET6;
                break;
            default:
                return 0;
        }
    }
    if(optind != argc){
        fputs("invalid arguments\n", stderr);
        return 0;
    }
    if(port == NULL){
        fputs("no port specified\n", stderr);
        return 0;
    }
    return 1;
}

int server_init(){
    struct addrinfo *addrs, *addr;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int rc = getaddrinfo(NULL, port, &hints, &addrs);
    if(rc != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return 0;
    }
    for(addr = addrs; addr != NULL; addr = addr->ai_next){
        server = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if(server == -1)
            continue;
        if(bind(server, addr->ai_addr, addr->ai_addrlen) != -1)
            break;
        close(server);
    }
    freeaddrinfo(addrs);
    if(addr == NULL)
        return 0;
    rc = listen(server, slots);
    if(rc == -1){
        perror("listen");
        close(server);
        return 0;
    }
    if(verbose)
        fprintf(stderr, "listening on %s\n", port);
    return 1;
}

int server_accept(){
    struct sockaddr_in incomer;
    socklen_t len = sizeof(incomer);
    memset(&incomer, 0, len);
    int fd = accept(server, (struct sockaddr*) &incomer, &len);
    if(fd == -1){
        perror("accept");
        return 0;
    }
    int i;
    for(i=0; i<slots; i++){
        if(clients[i] == -1){
            clients[i] = fd;
            if(verbose)
                fprintf(stderr, "incoming connection (fd %d)\n", fd);
            return 1;
        }
    }
    fputs("can't accept more connections\n", stderr);
    close(fd);
    return 0;
}

void shut(int *fd){
    if(*fd < 0)
        return;
    shutdown(*fd, SHUT_RDWR);
    close(*fd);
    if(verbose)
        fprintf(stderr, "connection closed (fd %d)\n", *fd);
    *fd = -1;
}

void clean_up(){
    close(server);
    int i;
    for(i=0; i<slots; i++)
        shut(clients+i);
    free(clients);
    exit(EXIT_SUCCESS);
}

int forward(int src){
    static char buffer[512];
    ssize_t sz = recv(src, &buffer, 512, 0);
    if(sz == -1)
        perror("recv");
    if(sz <= 0)
        return 0;
    int i;
    for(i=0; i<slots; i++){
        if(clients[i] != -1 && clients[i] != src){
            if(send(clients[i], buffer, sz, 0) == -1)
                perror("send");
        }
    }
    return 1;
}

#undef max
#define max(x,y) ((x) > (y) ? (x) : (y))

int main(int argc, char **argv){

    if(!parse_args(argc, argv))
        return EXIT_FAILURE;

    if(!server_init()){
        fputs("no address to bind\n", stderr);
        return EXIT_FAILURE;
    }
    clients = (int*) malloc(slots * sizeof(int));
    int i;
    for(i=0; i<slots; i++)
        clients[i] = -1;

    struct sigaction action;
    action.sa_handler = clean_up;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGHUP, &action, NULL);
    sigaction(SIGTERM, &action, NULL);

    int nfds, rc;
    fd_set rd;
    while(1){
        FD_ZERO(&rd);
        FD_SET(server, &rd);
        nfds = server;
        for(i=0; i<slots; i++){
            if(clients[i] >= 0){
                FD_SET(clients[i], &rd);
                nfds = max(nfds, clients[i]);
            }
        }
        rc = select(nfds+1, &rd, NULL, NULL, NULL);
        if(rc == -1 && errno == EINTR)
            continue;
        if(rc == -1){
            perror("select");
            break;
        }
        if(FD_ISSET(server, &rd))
            server_accept();
        for(i=0; i<slots; i++){
            if(FD_ISSET(clients[i], &rd)){
                if(!forward(clients[i]))
                    shut(clients+i);
            }
        }
    }

    clean_up();
    return EXIT_SUCCESS;

}
