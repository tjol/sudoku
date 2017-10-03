#ifndef _SUDOKU_H
#define _SUDOKU_H

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef DEBUG
#   define DEBUG 0
#endif

#if DEBUG
#   define _dbg(s) puts(s)
#   define _dbg_print_sudoku(s) print_sudoku(s, false)
#else
#   define _dbg(s)
#   define _dbg_print_sudoku(s)
#endif

typedef uint16_t field_t;
typedef field_t sudoku_t[9][9];

void print_sudoku(sudoku_t field, bool short_format);
bool all_are_fixed(sudoku_t field);
void fill_bits(const int number_field[9][9], sudoku_t bit_field);

int fill_sudoku_from_string(sudoku_t field, char *s);
int fill_sudoku_from_file(sudoku_t field, FILE *fp);

void clear_sudoku(sudoku_t field);

static inline bool is_fixed(field_t number)
{
    int i;
    for (i=0; i<9; ++i) {
        if (number == 1)
            return true;
        else if (number & 1)
            return false;
        else
            number >>= 1;
    }

    return false;
}

static inline int bits2number(field_t bits)
{
    int n;

    if (bits == 0) {
        return -1;
    } else {
        for (n=1; n<=9; ++n) {
            if (bits == 1) {
                return n;
                break;
            } else if (bits & 1) {
                return 0;
                break;
            }
            bits >>= 1;
        }
    }
    return -2;
}

static inline field_t number2bits(int n)
{
    if (n >= 1 && n <= 9)
        return 1 << (n-1);
    else
        return 0x1ff;
}

static inline int count_bits(field_t f)
{
    int count = 0;
    for (int i=0; i<9; ++i) {
        if (f & 1) count++;
        f >>= 1;
    }
    return count;
}

static inline int sudoku_cmp(sudoku_t s1, sudoku_t s2)
{
    return memcmp(s1, s2, sizeof(sudoku_t));
}

#endif /* _SUDOKU_H */
