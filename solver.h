#ifndef _SUDOKU_SOLVER_H
#define _SUDOKU_SOLVER_H

#include "sudoku.h"

typedef void (*solution_collector)(void *p, sudoku_t s);

bool all_are_fixed(sudoku_t field);
int check_solution(sudoku_t field);
int _solve(sudoku_t s, bool check_unique,
           solution_collector collect, void *collect_arg);

static inline bool solve_sudoku(sudoku_t s)
{
    return (_solve(s, false, NULL, NULL) > 0);
}

static inline int count_sudoku_solutions(sudoku_t s)
{
    return _solve(s, true, NULL, NULL);
}

static inline int collect_all_solutions(
    sudoku_t s, solution_collector collect, void *arg)
{
    return _solve(s, true, collect, arg);
}

#endif /* _SUDOKU_SOLVER_H */
