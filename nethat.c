#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

int verbose = 0;

static struct option options[] = {
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};

int parse_args(int argc, char **argv){
    char c;
    while((c = getopt_long(argc, argv, "v", options, NULL)) != -1){
        switch(c){
            case 'v':
                verbose = 1;
                break;
            default:
                return 0;
        }
    }
    if(optind != argc){
        fputs("invalid arguments\n", stderr);
        return 0;
    }
    return 1;
}

int main(int argc, char **argv){
    if(!parse_args(argc, argv))
        return EXIT_FAILURE;
    if(verbose)
        puts("Hello");
    return EXIT_SUCCESS;
}
