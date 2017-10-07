#include <stdio.h>
#include <ctype.h>
#include "sudoku.h"

bool all_are_fixed(sudoku_t field)
{
    int i, j;
    for (i=0; i<9; ++i) {
        for (j=0; j<0; ++j) {
            if (!is_fixed(field[i][j])) {
                return false;
            }
        }
    }
    return true;
}

void fill_bits(const int number_field[9][9], sudoku_t bit_field)
{
    int i, j;

    for (i=0; i<9; ++i) {
        for (j=0; j<9; ++j) {
            bit_field[i][j] = number2bits(number_field[i][j]);
        }
    }
}


void print_sudoku(sudoku_t field, bool short_format)
{
    int i, j;

    for (i=0; i<9; ++i) {
        for (j=0; j<9; ++j) {
            int n = bits2number(field[i][j]);
            switch(n) {
                case -1:
                    putchar('E');
                    break;
                case 0:
                    putchar('.');
                    break;
                case 1: 
                case 2: 
                case 3: 
                case 4: 
                case 5: 
                case 6: 
                case 7: 
                case 8: 
                case 9:
                    printf("%d", n);
                    break;
                default:
                    putchar('!');
                    break;
            }
            if (!short_format && j != 8)
                putchar(' ');
        }
        if (!short_format) putchar('\n');
    }
}

static inline field_t *insert_from_char(field_t *field_p, char c)
{
    if (isdigit(c)) {
        *(field_p++) = number2bits(c - '0');
    } else if (isspace(c)) {
        // ignore whitespace
    } else {
        // non-space non-digit:
        *(field_p++) = 0x1ff;
    }
    return field_p;
}

int fill_sudoku_from_string(sudoku_t field, char *s)
{
    field_t *field_p = (field_t*) field;
    field_t *end = field_p + 81;

    clear_sudoku(field);

    while (*s != 0 && field_p != end) {
        field_p = insert_from_char(field_p, *s);
        s++;
    }
    return field_p - ((field_t*) field);
}

int fill_sudoku_from_file(sudoku_t field, FILE *fp)
{
    char c;
    field_t *field_p = (field_t*) field;
    field_t *end = field_p + 81;

    clear_sudoku(field);

    while (((c = fgetc(fp)) != EOF) && field_p != end) {
        field_p = insert_from_char(field_p, c);
    }

    return field_p - ((field_t*) field);
}

void clear_sudoku(sudoku_t field)
{
    for (int i=0; i<9; ++i) {
        for (int j=0; j<9; ++j) {
            field[i][j] = 0x1ff;
        }
    }
}
