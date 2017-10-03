#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>

#include "sudoku.h"
#include "solver.h"

static bool all_solutions = false;
static bool count_solutions = true;
static bool short_output = false;
static int timeit_iters = 0;

static void process_sudoku_file(FILE *fp);

int main(int argc, char **argv)
{

    static struct option long_options[] = {
        {"help",              no_argument, 0, 'h'},
        {"all",               no_argument, 0, 'a'},
        {"count-solutions",   no_argument, 0, 'c'},
        {"do-not-count",      no_argument, 0, 'C'},
        {"short-output",      no_argument, 0, 's'},
        {"timeit",            required_argument, 0, 't'}
    };

    int c;
    while ((c = getopt_long(argc, argv, "hacCs", long_options, NULL))
                != -1) {
        switch (c) {
            case 'h':
                fprintf(stderr,
                    "Usage: %s [-h] [-aCcs] sudoku_file ...\n"
                    "\n"
                    "Options:\n"
                    "    --help -h\n"
                    "        Display this help message\n"
                    "    --count-solutions -c\n"
                    "        Count how many solutions there are (default)\n"
                    "    --do-not-count -C\n"
                    "        Do not count how many solutions there are\n"
                    "    --all -a\n"
                    "        Find all solutions to the puzzles (overrides -c, -C)\n"
                    "    --short-output -s\n"
                    "        Use a shorter output format.\n"
                    "    --timeit=iterations\n"
                    "        Time the solver.\n",
                    argv[0]);
                return 0;
            case 'c':
                count_solutions = true;
                break;
            case 'C':
                count_solutions = false;
                break;
            case 'a':
                all_solutions = true;
                break;
            case 's':
                short_output = true;
                break;
            case 't':
                timeit_iters = atoi(optarg);
                break;
            default:
                return 2;
        }
    }

    if (optind == argc) {
        process_sudoku_file(stdin);
    } else {
        for (int i=optind; i<argc; ++i) {
            char *fn = argv[i];
            FILE *fp;
            if (strcmp(fn, "-") == 0) {
                fp = stdin;
            } else {
                fp = fopen(fn, "r");
                if (!fp) {
                    fprintf(stderr, "Error opening %s: ", fn);
                    perror(NULL);
                    return 1;
                }
            }

            process_sudoku_file(fp);
        }
    }
}

struct solutions_list;
struct solutions_list {
    sudoku_t field;
    struct solutions_list *next;
};

struct solutions_list *new_solutions_list()
{
    struct solutions_list *lst = malloc(sizeof(struct solutions_list));
    lst->next = NULL;
    return lst;
}

void save_solution(struct solutions_list *lst, sudoku_t s)
{
    while (lst->next)
        lst = lst->next;

    memcpy(lst->field, s, sizeof(sudoku_t));
    lst->next = new_solutions_list();
}

void free_solutions_list(struct solutions_list *lst)
{
    if (lst->next)
        free_solutions_list(lst->next);
    free(lst);
}

#define TIMEIT(VAR, STMT) \
    { \
        struct timeval _TIMEIT_t0, _TIMEIT_t1; \
        gettimeofday(&_TIMEIT_t0, 0); \
        for (int _TIMEIT_iter = 0; _TIMEIT_iter < timeit_iters; ++_TIMEIT_iter) { \
            STMT; \
        } \
        gettimeofday(&_TIMEIT_t1, 0); \
        VAR = (_TIMEIT_t1.tv_sec - _TIMEIT_t0.tv_sec) * 1000.0 \
              + (_TIMEIT_t1.tv_usec - _TIMEIT_t0.tv_usec) / 1000.0; \
    }

void process_sudoku_file(FILE *fp)
{
    sudoku_t s, buffer;
    double dt_ms = 0;

    while (fill_sudoku_from_file(s, fp) > 0) {
        if (!short_output) {
            puts("Sudoku: ");
            print_sudoku(s, false);
            puts("");
        }

        if (count_solutions || all_solutions) {
            int solution_count = 0;
            struct solutions_list *solutions;
            solutions = new_solutions_list();

            if (timeit_iters == 0) {
                if (all_solutions) {
                    solution_count = collect_all_solutions(s,
                        (solution_collector)save_solution, solutions);
                } else {
                    solution_count = count_sudoku_solutions(s);
                }
            } else {
                memcpy(buffer, s, sizeof(sudoku_t));
                TIMEIT(dt_ms, memcpy(s, buffer, sizeof(sudoku_t));
                              if (all_solutions) {
                                  free_solutions_list(solutions);
                                  solutions = new_solutions_list();
                                  solution_count = collect_all_solutions(s, 
                                    (solution_collector)save_solution, solutions);
                              } else {
                                  solution_count = count_sudoku_solutions(s);
                              })
            }

            if (short_output) {
                if (solution_count != 0) {
                    print_sudoku(s, true);
                    if (timeit_iters)
                        printf(" %d %f\n", solution_count, dt_ms/timeit_iters);
                    else
                        printf(" %d\n", solution_count);
                } else {
                    printf("no solution\n");
                }
            } else {
                if (solution_count != 0) {
                    if (solution_count == 1)
                        printf("\nThere is 1 solution.\n");
                    else
                        printf("\nThere are %d solutions.\n", solution_count);
                    
                    if (all_solutions) {
                        struct solutions_list *lst = solutions;
                        while (lst->next) {
                            print_sudoku(lst->field, false);
                            puts("");
                            lst = lst->next;
                        }
                    } else print_sudoku(s, false);
                } else {
                    printf("\nThere are no solutions.\n");
                }

                if (timeit_iters)
                    printf("Running time %.2f s (%.2f ms per iteration)\n", dt_ms/1000.0, dt_ms/timeit_iters);

                if (all_solutions)
                    free_solutions_list(solutions);
            }
        } else {
            bool solved = false;
            if (timeit_iters == 0)
                solved = solve_sudoku(s);
            else {
                memcpy(buffer, s, sizeof(sudoku_t));
                TIMEIT(dt_ms, memcpy(s, buffer, sizeof(sudoku_t));
                              solved = solve_sudoku(s);)
            }
            if (solved) {
                print_sudoku(s, short_output);
                if (timeit_iters) {
                    if (short_output)
                        printf(" %f", dt_ms/timeit_iters);
                    else
                        printf("Running time %.2f s (%.2f ms per iteration)\n", dt_ms/1000.0, dt_ms/timeit_iters);
                }
                printf("\n");
            } else {
                printf("no solution");
                if (timeit_iters) {
                    if (short_output)
                        printf(" %f", dt_ms/timeit_iters);
                    else
                        printf("Running time %.2f s (%.2f ms per iteration)\n", dt_ms/1000.0, dt_ms/timeit_iters);
                }
                printf("\n");
            }
        }
    }
}
