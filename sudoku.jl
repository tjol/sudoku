#!/usr/bin/env julia

using IterTools
using ArgParse

SudokuField() = IntSet([1,2,3,4,5,6,7,8,9])
SudokuField(n) = (1 <= n <= 9) ? IntSet([n]) : error("invalid sudoku field")
Sudoku() = fill(SudokuField(), (9,9))

field_is_fixed(f) = (length(f) == 1)


impose_num!(sudoku, i, j, f::IntSet) = impose_num!(sudoku, i, j, collect(f)[1])

row_of(i,j) = ((i,k) for k in 1:9)
col_of(i,j) = ((k,j) for k in 1:9)
block_of(i,j) = ((k,l) for k in 3*div(i-1,3)+1:3*div(i-1,3)+3
                       for l in 3*div(j-1,3)+1:3*div(j-1,3)+3)

function impose_num!(sudoku, i, j, n::Integer)
    newly_fixed = Set{Tuple{Int,Int}}()

    sudoku[i,j] = SudokuField(n)

    @itr for (k,l) in chain(col_of(i,j), row_of(i,j), block_of(i,j))
        if k == i && j == l
            continue
        end
        was_fixed = field_is_fixed(sudoku[k,l])
        sudoku[k,l] = setdiff(sudoku[k,l], [n])
        if !was_fixed && field_is_fixed(sudoku[k,l])
            push!(newly_fixed, (k,l))
        end
    end

    for (k,l) in newly_fixed
        if !field_is_fixed(sudoku[k,l])
            continue
        end
        impose_num!(sudoku, k, l, sudoku[k,l])
    end
end

function eliminate_in_region!(sudoku, indexpairs)
    imposed = 0

    for (i,j) in indexpairs
        f = copy(sudoku[i,j])

        if field_is_fixed(f)
            continue
        end

        for (k,l) in indexpairs
            if k == i && l == j
                continue
            end
            setdiff!(f, sudoku[k,l])
        end

        if length(f) == 1
            impose_num!(sudoku, i, j, f)
            imposed += 1
        end
    end

    return imposed
end

function do_elimination!(sudoku)
    imposed = 0
    for i in 1:9
        imposed += eliminate_in_region!(sudoku, collect(row_of(i, i)))
        imposed += eliminate_in_region!(sudoku, collect(col_of(i, i)))
        imposed += eliminate_in_region!(sudoku, collect(block_of(((i-1)%3)*3+1, div(i-1,3)*3+1)))
    end

    if imposed != 0
        do_elimination!(sudoku)
    end
end

function solve!(sudoku, find_all_solutions=true, callback=nothing)
    do_elimination!(sudoku)

    status = check_solution(sudoku)
    if status == SUDOKU_DONE
        if callback != nothing
            callback(sudoku)
        end
        return 1
    elseif status == SUDOKU_ERROR
        return 0
    else
        # We'll have to do some guessing

        # Find the lowest-order point
        lowest_order = 9
        lowest_i = 0
        lowest_j = 0
        for j in 1:9
            for i in 1:9
                f = sudoku[i,j]
                if 1 < length(f) < lowest_order
                    lowest_order = length(f)
                    lowest_i = i
                    lowest_j = j
                    if length(f) == 2
                        break
                    end
                end
            end
        end

        options = sudoku[lowest_i,lowest_j]
        solution = nothing
        solutions_count = 0

        for n in options
            trial_sudoku = copy(sudoku)
            impose_num!(trial_sudoku, lowest_i, lowest_j, n)
            solutions_here = solve!(trial_sudoku, find_all_solutions, callback)
            solutions_count += solutions_here
            if solutions_here > 0
                solution = copy(trial_sudoku)

                if !find_all_solutions
                    break
                end
            end
        end

        if solutions_count != 0
            sudoku[:] = solution[:]
        end
        return solutions_count
    end
end

@enum SudokuStatus SUDOKU_ERROR SUDOKU_DONE SUDOKU_IN_PROGRESS

function check_solution(sudoku)
    any_not_fixed = false
    for f in sudoku
        if length(f) == 0
            return SUDOKU_ERROR
        elseif !field_is_fixed(f)
            any_not_fixed = true;
        end
    end
    return any_not_fixed ? SUDOKU_IN_PROGRESS : SUDOKU_DONE
end


function Sudoku(nums::Matrix{T}) where {T<:Integer}
    if size(nums) != (9,9)
        error("sudoku are 9x9")
    end

    s = Sudoku()
    for j in 1:9
        for i in 1:9
            n = nums[i,j]
            if 1 <= n <= 9
                impose_num!(s, i, j, n)
            end
        end
    end
    return s
end

function read_nums(stream::IO)
    m = zeros(Int, (9,9))
    for i in 1:9
        for j in 1:9
            while true
                c = 0
                try
                    c = read(stream, Char)
                catch EOFError
                    return m
                end

                if isdigit(c)
                    m[i,j] = parse(Int, c)
                    break
                elseif isspace(c)
                    # skip spaces
                else
                    break
                end
            end
        end
    end
    return m
end

read_nums(s::String) = read_nums(IOBuffer(s))

function print_sudoku(s, short=false)
    for i in 1:9
        for j in 1:9
            f = s[i,j]
            print(field_is_fixed(f) ? collect(f)[1] : '.')
            if !short
                print(j == 9 ? '\n' : ' ')
            end
        end
    end
end

function main()
    s = ArgParseSettings()
    @add_arg_table s begin
        "--count-solutions", "-c"
            help = "Count how many solutions there are (default)"
            dest_name = "count-solutions"
            action = :store_true
        "--do-not-count", "-C"
            help = "Do not count how many solutions there are"
            dest_name = "count-solutions"
            action = :store_false
            force_override = true
        "--all", "-a"
            help = "Find all solutions to the puzzle"
            action = :store_true
        "--short-output", "-s"
            help = "Use a shorter output format"
            action = :store_true
        "--timeit"
            help = "Time the solver"
            metavar = "ITERATIONS"
            arg_type = Int
            default = 0
        "sudoku_file"
            help = "input file(s)"
            nargs = '*'

    end

    args = parse_args(s)

    my_print_sudoku(s) = (print_sudoku(s, args["short-output"]);
                          args["short-output"] ? nothing : print("\n"))

    sudoku_files = args["sudoku_file"]
    if length(sudoku_files) == 0
        sudoku_files = ["-"]
    end

    for fn in sudoku_files
        fp = STDIN
        if fn != "-"
           fp = open(fn, "r") 
        end

        while true

            sudoku_nums = read_nums(fp)
            if all(sudoku_nums .== 0)
                exit(0)
            end

            if args["timeit"] > 0
                t1 = time() 
                n = 0
                for i in 1:args["timeit"]
                    s = Sudoku(sudoku_nums)
                    n = solve!(s, args["count-solutions"])
                end
                t2 = time()
                dt_ms = (t2 - t1)*1000
                my_print_sudoku(s)
                if args["short-output"]
                    if args["count-solutions"]
                        @printf " %d" n
                    end
                    @printf " %f\n" dt_ms/args["timeit"]
                else
                    if args["count-solutions"]
                        if n > 1
                            @printf "There are %d solutions.\n" n
                        elseif n == 1
                            print("There is 1 solution.\n")
                        else
                            print("There are no solutions.\n")
                        end
                    end
                    @printf "Running time %.2f s (%.2f ms per iteration)\n" dt_ms dt_ms/args["timeit"]
                end
            else
                s = Sudoku(sudoku_nums)
                n = 0
                if args["all"]
                    n = solve!(s, true, args["short-output"] ? nothing : my_print_sudoku)
                    if args["short-output"]
                        my_print_sudoku(s)
                    end
                else
                    n = solve!(s, args["count-solutions"])
                    my_print_sudoku(s)
                end

                if args["short-output"]
                    if args["count-solutions"]
                        @printf " %d\n" n
                    end
                else
                    if args["count-solutions"]
                        if n > 1
                            @printf "There are %d solutions.\n" n
                        elseif n == 1
                            print("There is 1 solution.\n")
                        else
                            print("There are no solutions.\n")
                        end
                    end
                end
            end
        end
    end
end

main()

