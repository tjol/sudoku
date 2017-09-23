#include <string.h>
#include <stdlib.h>
#include "solver.h"

static inline void remove_option(field_t number, field_t *place)
{
    *place &= ~number;
}

static void impose(sudoku_t field, int i, int j, bool recurse);
static inline void impose(sudoku_t field, int i, int j, bool recurse)
{
    int k, l;
    int ii_now_fixed[81];
    int jj_now_fixed[81];
    int newly_fixed_count = 0;

    // impose the constraint of location [i,j] on its peers
    field_t f = field[i][j];
    if (!is_fixed(f)) return;

    for (k=0; k<9; ++k) {
        // Check my row!
        if (k != j){
            bool was_fixed = is_fixed(field[i][k]);
            remove_option(f, &field[i][k]);

            if (recurse && !was_fixed && is_fixed(field[i][k])) {
                ii_now_fixed[newly_fixed_count] = i;
                jj_now_fixed[newly_fixed_count] = k;
                newly_fixed_count++;
            }
        }
        // Check my column!
        if (k != i){
            bool was_fixed = is_fixed(field[k][j]);
            remove_option(f, &field[k][j]);

            if (recurse && !was_fixed && is_fixed(field[k][j])) {
                ii_now_fixed[newly_fixed_count] = k;
                jj_now_fixed[newly_fixed_count] = j;
                newly_fixed_count++;
            }
        }
    }

    // check my corner!
    int origin1 = (i/3)*3;
    int origin2 = (j/3)*3;
    for (k=origin1; k<origin1+3; ++k) {
        for (l=origin2; l<origin2+3; ++l) {
            if (k != i || l != j) {
                bool was_fixed = is_fixed(field[k][l]);
                remove_option(f, &field[k][l]);

                if (recurse && !was_fixed && is_fixed(field[k][l])) {
                    ii_now_fixed[newly_fixed_count] = k;
                    jj_now_fixed[newly_fixed_count] = l;
                    newly_fixed_count++;
                }
            }
        }
    }

    // Recursively fix new constraints
    for (int m=0; m<newly_fixed_count; ++m) {
        impose(field, ii_now_fixed[m], jj_now_fixed[m], true);
    }
}

static void iterate_sudoku(sudoku_t field)
{
    int i, j;

    for (i=0; i<9; ++i) {
        for (j=0; j<9; ++j) {
            impose(field, i, j, false);
        }
    }
}

static void iterate_elimination(sudoku_t field)
{
    int i, j, k, l;

    bool imposed_any;

    do {
        imposed_any = false;
        for (i=0; i<9; ++i) {
            for (j=0; j<9; ++j) {
                if (is_fixed(field[i][j])) continue;

                // Go through the row, column, and corner.
                // See if there is any number that can only be here.
                field_t row_mask = 0, col_mask = 0, corner_mask = 0;
                for (k=0; k<9; ++k) {
                    if (k != j) row_mask |= field[i][k];
                    if (k != i) col_mask |= field[k][j];
                }

                field_t row_unique = field[i][j] & (~row_mask);
                if (is_fixed(row_unique)) {
                    field[i][j] = row_unique;
                    impose(field, i, j, true);
                    imposed_any = true;
                    continue;
                }

                field_t col_unique = field[i][j] & (~col_mask);
                if (is_fixed(col_unique)) {
                    field[i][j] = col_unique;
                    impose(field, i, j, true);
                    imposed_any = true;
                    continue;
                }

                // check my corner!
                int origin1 = (i/3)*3;
                int origin2 = (j/3)*3;
                for (k=origin1; k<origin1+3; ++k) {
                    for (l=origin2; l<origin2+3; ++l) {
                        if (k != i || l != j) {
                            corner_mask |= field[k][l];
                        }
                    }
                }

                field_t corner_unique = field[i][j] & (~corner_mask);
                if (is_fixed(corner_unique)) {
                    field[i][j] = corner_unique;
                    impose(field, i, j, true);
                    imposed_any = true;
                    continue;
                }
            }
        }
    } while(imposed_any);
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

int _solve_more(sudoku_t s, bool check_unique,
           solution_collector collect, void *collect_arg)
{
    sudoku_t buffer, a_solution;

    for(;;) {

        iterate_elimination(s);

        switch (check_solution(s)) {
            case SUDOKU_DONE:
                _dbg("DONE\n");
                if (collect != NULL)
                    (*collect)(collect_arg, s);
                return 1;
            case SUDOKU_ERROR:
                _dbg("ERROR\n");
                return 0;
            default:
                _dbg("CONTINUE\n");
        }

        memcpy(buffer, s, sizeof(sudoku_t));

        // Guess something!
        // What shall we guess?
        int simplest_i, simplest_j;
        int simplest_n_bits = 10;

        for (int i=0; i<9; ++i) {
            for (int j=0; j<9; ++j) {
                int count = count_bits(buffer[i][j]);
                if (count < simplest_n_bits && count > 1) {
                    simplest_n_bits = count;
                    simplest_i = i;
                    simplest_j = j;
                }
            }
        }

        if (simplest_n_bits == 10) {
            return 0;
        }

        // We've selected the earliest point with the lowest number
        // of possibilities. Try all.

        int my_solutions_count = 0;

        for (int i=0; i<9; ++i) {
            if ((s[simplest_i][simplest_j] >> i) & 1) {
                buffer[simplest_i][simplest_j] = (1 << i);
                impose(buffer, simplest_i, simplest_j, true)

                _dbg("HAVE \n");
                _dbg_print_sudoku(s);
                _dbg("GUESS \n");
                _dbg_print_sudoku(buffer);
                // The buffer now contains our guess
                int solutions_here = _solve_more(buffer, check_unique,
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

int _solve(sudoku_t s, bool check_unique,
           solution_collector collect, void *collect_arg)
{
    _dbg("Solving:\n");
    _dbg_print_sudoku(s);
    iterate_sudoku(s);

    return _solve_more(s, check_unique, collect, collect_arg);
}
