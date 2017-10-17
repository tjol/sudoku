#!/usr/bin/env python3


#############################################
# START OF NORVIG CODE
# From https://github.com/norvig/pytudes/blob/master/sudoku.py
#############################################


## Solve Every Sudoku Puzzle

## See http://norvig.com/sudoku.html

## Throughout this program we have:
##   r is a row,    e.g. 'A'
##   c is a column, e.g. '3'
##   s is a square, e.g. 'A3'
##   d is a digit,  e.g. '9'
##   u is a unit,   e.g. ['A1','B1','C1','D1','E1','F1','G1','H1','I1']
##   grid is a grid,e.g. 81 non-blank chars, e.g. starting with '.18...7...
##   values is a dict of possible values, e.g. {'A1':'12349', 'A2':'8', ...}

def cross(A, B):
    "Cross product of elements in A and elements in B."
    return [a+b for a in A for b in B]

digits   = '123456789'
rows     = 'ABCDEFGHI'
cols     = digits
squares  = cross(rows, cols)
unitlist = ([cross(rows, c) for c in cols] +
            [cross(r, cols) for r in rows] +
            [cross(rs, cs) for rs in ('ABC','DEF','GHI') for cs in ('123','456','789')])
units = dict((s, [u for u in unitlist if s in u])
             for s in squares)
peers = dict((s, set(sum(units[s],[]))-set([s]))
             for s in squares)

################ Unit Tests ################

def test():
    "A set of tests that must pass."
    assert len(squares) == 81
    assert len(unitlist) == 27
    assert all(len(units[s]) == 3 for s in squares)
    assert all(len(peers[s]) == 20 for s in squares)
    assert units['C2'] == [['A2', 'B2', 'C2', 'D2', 'E2', 'F2', 'G2', 'H2', 'I2'],
                           ['C1', 'C2', 'C3', 'C4', 'C5', 'C6', 'C7', 'C8', 'C9'],
                           ['A1', 'A2', 'A3', 'B1', 'B2', 'B3', 'C1', 'C2', 'C3']]
    assert peers['C2'] == set(['A2', 'B2', 'D2', 'E2', 'F2', 'G2', 'H2', 'I2',
                               'C1', 'C3', 'C4', 'C5', 'C6', 'C7', 'C8', 'C9',
                               'A1', 'A3', 'B1', 'B3'])
    print('All tests pass.')

################ Parse a Grid ################

def parse_grid(grid):
    """Convert grid to a dict of possible values, {square: digits}, or
    return False if a contradiction is detected."""
    ## To start, every square can be any digit; then assign values from the grid.
    values = dict((s, digits) for s in squares)
    for s,d in grid_values(grid).items():
        if d in digits and not assign(values, s, d):
            return False ## (Fail if we can't assign d to square s.)
    return values

def grid_values(grid):
    "Convert grid into a dict of {square: char} with '0' or '.' for empties."
    chars = [c for c in grid if c in digits or c in '0.']
    assert len(chars) == 81
    return dict(zip(squares, chars))

################ Constraint Propagation ################

def assign(values, s, d):
    """Eliminate all the other values (except d) from values[s] and propagate.
    Return values, except return False if a contradiction is detected."""
    other_values = values[s].replace(d, '')
    if all(eliminate(values, s, d2) for d2 in other_values):
        return values
    else:
        return False

def eliminate(values, s, d):
    """Eliminate d from values[s]; propagate when values or places <= 2.
    Return values, except return False if a contradiction is detected."""
    if d not in values[s]:
        return values ## Already eliminated
    values[s] = values[s].replace(d,'')
    ## (1) If a square s is reduced to one value d2, then eliminate d2 from the peers.
    if len(values[s]) == 0:
        return False ## Contradiction: removed last value
    elif len(values[s]) == 1:
        if not all(eliminate(values, s2, values[s]) for s2 in peers[s]):
            return False
    ## (2) If a unit u is reduced to only one place for a value d, then put it there.
    for u in units[s]:
        dplaces = [s for s in u if d in values[s]]
        if len(dplaces) == 0:
            return False ## Contradiction: no place for this value
        elif len(dplaces) == 1:
            # d can only be in one place in unit; assign it there
            if not assign(values, dplaces[0], d):
                return False
    return values


################ Search ################

def solve(grid):
    return search(parse_grid(grid))

def search(values):
    "Using depth-first search and propagation, try all possible values."
    if values is False:
        return False ## Failed earlier
    if all(len(values[s]) == 1 for s in squares):
        return values ## Solved!
    ## Chose the unfilled square s with the fewest possibilities
    n,s = min((len(values[s]), s) for s in squares if len(values[s]) > 1)
    return some(search(assign(values.copy(), s, d)) for d in values[s])

################ Utilities ################

def some(seq):
    "Return some element of seq that is true."
    for e in seq:
        if e: return e
    return False

def from_file(filename, sep='\n'):
    "Parse a file into a list of strings, separated by sep."
    return file(filename).read().strip().split(sep)

#############################################
# END OF NORVIG CODE
#############################################


# command-line UI vaguely compatible with the other language versions

import sys
import argparse
import time

def print_grid(grid, short_output=False):
    space = '' if short_output else ' '
    newline = '' if short_output else '\n'

    if isinstance(grid, dict):
        # This is a (partial) solution
        for r in rows:
            for c in cols:
                v = grid[r+c]
                if len(v) == 1:
                    sys.stdout.write(v)
                else:
                    sys.stdout.write('.')
                if c != '9':
                    sys.stdout.write(space)
            sys.stdout.write(newline)
    else:
        indices = [(i,j) for i in range(9) for j in range(9)]
        for n, (i, j) in zip(grid, indices):
            sys.stdout.write(n if n in digits else '.')
            if j != 8:
                sys.stdout.write(space)
            else:
                sys.stdout.write(newline)

    sys.stdout.flush()

def grid_from_file(fp):
    grid = []
    while True:
        c = fp.read(1)
        if not c:
            raise EOFError

        if c.isspace():
            continue

        if c in digits:
            grid.append(c)
        else:
            grid.append('.')

        if len(grid) == 81:
            return grid

def main():
    parser = argparse.ArgumentParser(description='Sudoku solver (Python)')
    parser.add_argument('sudoku_files', type=str, nargs='*',
                        help='input file(s)')
    parser.add_argument('--short-output', '-s',
                        help='Use a shorter output format',
                        action='store_true')
    parser.add_argument('--timeit', metavar='ITERATIONS', type=int,
                        default=0)

    args = parser.parse_args()

    filenames = args.sudoku_files or ['-']

    for fn in filenames:
        if fn == '-':
            process_sudoku_file(sys.stdin, args.short_output, args.timeit)
        else:
            with open(fn, 'r') as fp:
                process_sudoku_file(fp, args.short_output, args.timeit)

    return 0

def process_sudoku_file(fp, short_output=False, timeit_iters=0):
    while True:
        try:
            grid = grid_from_file(fp)
            if not short_output:
                print('Sudoku:')
                print_grid(grid)
                print()

            if timeit_iters == 0:
                solved_grid = solve(grid)

                print_grid(solved_grid, short_output=short_output)
                print()
            else:
                t1 = time.perf_counter()
                for _ in range(timeit_iters):
                    solved_grid = solve(grid)
                t2 = time.perf_counter()
                dt_ms = (t2 - t1) * 1000

                print_grid(solved_grid, short_output=short_output)
                if short_output:
                    print(' %f' % (dt_ms/timeit_iters))
                else:
                    print('Running time %.2f s (%.2f ms per iteration)' %
                          (dt_ms, dt_ms/timeit_iters))

        except (ValueError, EOFError, IOError):
            return





if __name__ == '__main__':
    sys.exit(main())
