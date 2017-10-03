override CFLAGS := -W -Wall -std=c99 -pedantic -O1 -g $(CFLAGS)

DEPS = sudoku.h solver.h

all: sudoku gen-sudoku

sudoku: sudoku_main.o sudoku.o solver.o
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $^

gen-sudoku: sudoku.o solver.o generator.o generate_main.o
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	gcc -c -o $@ $< $(CFLAGS)

clean:
	rm -vf *.o sudoku

.PHONY: clean all 
