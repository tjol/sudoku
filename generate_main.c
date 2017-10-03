#include "generator.h"
#include <stdlib.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    bool short_output = false;

    srand(time(NULL));

    int n_sudoku = 1;

    static struct option long_options[] = {
        {"help",              no_argument, 0, 'h'},
        {"short-output",      no_argument, 0, 's'},
        {"seed",              required_argument, 0, 'S'}
    };

    int c;
    while ((c = getopt_long(argc, argv, "hsS:", long_options, NULL))
                != -1) {
        switch (c) {
            case 'h':
                fprintf(stderr,
                    "Usage: %s [-h] [-s] [-S seed] count\n"
                    "\n"
                    "Options:\n"
                    "    --help -h\n"
                    "        Display this help message\n"
                    "    --short-output -s\n"
                    "        Use a shorter output format.\n"
                    "    --seed=seed -S seed\n"
                    "        Initialize the random number generator with seed.\n",
                    argv[0]);
                return 0;
            case 's':
                short_output = true;
                break;
            case 'S':
                srand(atoi(optarg));
                break;
            default:
                return 2;
        }
    }

    if ((argc - optind) == 1) {
        n_sudoku = atoi(argv[optind]);
    } else if ((argc - optind) > 1) {
        fprintf(stderr, "ERROR: too many arguments\n");
        return 2;
    }

    sudoku_t s;
    
    for (int i=0; i<n_sudoku; ++i) {
        if (!short_output && i != 0) {
            puts("");
        }
        generate_sudoku(s);
        print_sudoku(s, short_output);
        if(short_output) puts("");
    }
    return 0;
}