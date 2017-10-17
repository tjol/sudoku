#include "generator.h"
#include "solver.h"

#include <stdlib.h>

static inline field_t random_allowed(field_t current)
{
    for(;;) {
        field_t trial = 1 << (rand() % 9);
        // Is this allowed?
        if ((current & trial) != 0)
            return trial;
    }
}

static bool remove_random(sudoku_t s);

void generate_sudoku(sudoku_t buffer)
{
    int i, j;
    sudoku_t solve_buffer;

    // generate an empty sudoku
    for (i=0; i<9; ++i)
        for (j=0; j<9; ++j)
            buffer[i][j] = 0x1ff;

    // Set random values!
    for (i=0; i<9; ++i) {
        for (j=0; j<9; ++j) {
            sudoku_t prev;
            memcpy(prev, buffer, sizeof(sudoku_t));

            if (is_fixed(buffer[i][j])) continue;
            buffer[i][j] = random_allowed(buffer[i][j]);
            impose(buffer, i, j, true);

            memcpy(solve_buffer, buffer, sizeof(sudoku_t));
            // Is there a solution here?
            if (!solve_sudoku(solve_buffer)) {
                memcpy(buffer, prev, sizeof(sudoku_t));
                if ((--j) < 0) {
                    --i;
                    j = 8;
                }
            }
        }
    }

    while(remove_random(buffer));
}

static bool remove_random(sudoku_t s)
{
    sudoku_t buffer;
    sudoku_t solve_buffer;

    int can_remove[9][9];
    int i, j;
    int can_remove_count = 0;

    for (i=0; i<9; ++i) {
        for (j=0; j<9; ++j) {
            can_remove[i][j] = is_fixed(s[i][j]);
            if (can_remove[i][j])
                can_remove_count++;
        }
    }


    while (can_remove_count > 0) {
        // find something that I can remove, at random.
        do {
            i = rand() % 9;
            j = rand() % 9;
        } while(!can_remove[i][j]);

        // remove it
        memcpy(buffer, s, sizeof(sudoku_t));
        buffer[i][j] = 0x1ff;
        memcpy(solve_buffer, buffer, sizeof(sudoku_t));

        // did that work?
        if (count_sudoku_solutions(solve_buffer) == 1) {
            // Excellent.
            memcpy(s, buffer, sizeof(sudoku_t));
            return true;
        } else {
            // No. Try removing something else.
            can_remove[i][j] = 0;
            can_remove_count--;
        }
    }

    // We exhausted our possibilities without being able to remove anything.
    // This is the minimal sudoku.
    return false;
}
