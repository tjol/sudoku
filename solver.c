#include <string.h>
#include <stdlib.h>
#include "solver.h"

static inline void remove_option(field_t number, field_t *place)
{
    *place &= ~number;
}

static bool iterate_sudoku(sudoku_t field)
{
    int i, j, k, l;
    int corner1, corner2;

    // process rows
    for (i=0; i<9; ++i) {
        for (j=0; j<9; ++j) {
            if (is_fixed(field[i][j])) {
                for (k=0; k<9; ++k) {
                    if (k != j) remove_option(field[i][j], &field[i][k]);
                }
            }
        }
    }

    // process columns
    for (i=0; i<9; ++i) {
        for (j=0; j<9; ++j) {
            if (is_fixed(field[j][i])) {
                for (k=0; k<9; ++k) {
                    if (k != j) remove_option(field[j][i], &field[k][i]);
                }
            }
        }
    }

    // Process sectors
    for (corner1=0; corner1<9; corner1+=3) {
        for (corner2=0; corner2<9; corner2+=3) {
            for (i=0; i<3; ++i) {
                for (j=0; j<3; ++j) {
                    if (is_fixed(field[corner1+i][corner2+j])) {
                        for (k=0; k<3; ++k) {
                            for (l=0; l<3; ++l) {
                                if (k != i || l != j)
                                    remove_option(field[corner1+i][corner2+j],
                                                 &field[corner1+k][corner2+l]);
                            }
                        }
                    }
                }
            }
        }
    }

    return all_are_fixed(field);
}

int check_solution(sudoku_t field)
{
    int i, j;

    bool any_not_fixed = false;

    for (i=0; i<9; ++i) {
        for (j=0; j<9; ++j) {
            if (field[i][j] == 0) {
                return SUDOKU_ERROR;
            } else if (!is_fixed(field[i][j])) {
                any_not_fixed = true;
            }
        }
    }

    if (any_not_fixed) return SUDOKU_IN_PROGRESS;
    else return SUDOKU_DONE;
}

int _solve(sudoku_t s, bool check_unique,
           solution_collector collect, void *collect_arg)
{
    sudoku_t buffer, a_solution;
    memcpy(buffer, s, sizeof(sudoku_t));
    
    _dbg("Solving:\n");
    _dbg_print_sudoku(s);

    for(;;) {
        _dbg("\nIterating!\n");
        iterate_sudoku(buffer);

        if (sudoku_cmp(s, buffer) != 0) {
            _dbg("Progress was made.\n");
            _dbg_print_sudoku(buffer);
            memcpy(s, buffer, sizeof(sudoku_t));

        } else {
            // The last iteration was stable.

            switch (check_solution(buffer)) {
                case SUDOKU_DONE:
                    _dbg("DONE\n");
                    if (collect != NULL)
                        (*collect)(collect_arg, buffer);
                    memcpy(s, buffer, sizeof(sudoku_t));
                    return 1;
                case SUDOKU_ERROR:
                    _dbg("ERROR\n");
                    return 0;
                default:
                    _dbg("CONTINUE\n");
            }

            // ... but it didn't yield a final result.

            // Guess something!
            // What shall we guess?
            field_t *simplest = NULL;
            field_t *simplest_buf = NULL;
            int simplest_n_bits = 10;

            for (int i=0; i<9; ++i) {
                for (int j=0; j<9; ++j) {
                    int count = count_bits(buffer[i][j]);
                    if (count < simplest_n_bits && count > 1) {
                        simplest_n_bits = count;
                        simplest = &s[i][j];
                        simplest_buf = &buffer[i][j];
                    }
                }
            }

            if (simplest_n_bits == 10) {
                return 0;
            }

            // We've selected the earliest point with the lowest number
            // of possibilities. Try all.
            memcpy(s, buffer, sizeof(sudoku_t));

            int my_solutions_count = 0;

            for (int i=0; i<9; ++i) {
                if (((*simplest) >> i) & 1) {
                    *simplest_buf = (1 << i);

                    _dbg("HAVE \n");
                    _dbg_print_sudoku(s);
                    _dbg("GUESS \n");
                    _dbg_print_sudoku(buffer);
                    // The buffer now contains our guess
                    int solutions_here = _solve(buffer, check_unique,
                                                collect, collect_arg);
                    if (solutions_here > 0) {
                        // done!
                        
                        if (!check_unique) {
                            memcpy(s, buffer, sizeof(sudoku_t));
                            return 1;
                        } else {
                            my_solutions_count += solutions_here;
                            memcpy(a_solution, buffer, sizeof(sudoku_t));
                            // backtrack to find more solutions!
                            memcpy(buffer, s, sizeof(sudoku_t));
                        }
                    } else {
                        // backtrack!
                        memcpy(buffer, s, sizeof(sudoku_t));
                    }
                    _dbg("... next guess\n");
                }
            }

            if (my_solutions_count == 0) _dbg("Dead end.\n");

            memcpy(s, a_solution, sizeof(sudoku_t));

            return my_solutions_count;
        }
    }
}

