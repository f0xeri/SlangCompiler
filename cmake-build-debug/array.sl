module f1
start
    // create 3d array of size 10x10x10 and fill with 0
    variable-array[10] array[10] array[10] integer arr;
    variable-integer i := 0;
    variable-integer j := 0;
    variable-integer k := 0;
    while 0 repeat
        if 0 then
        end if;
    end while;
    while i < 10 repeat
        while j < 10 repeat
            while k < 10 repeat
                arr[i][j][k] := 0;
                k := k + 1;
            end while;
            j := j + 1;
            k := 0;
        end while;
        i := i + 1;
        j := 0;
    end while;

    // create matrix and fill it with 0
    variable-array[10] array[10] integer matrix;
    variable-integer i := 0;
    variable-integer j := 0;
    while i < 10 repeat
        while j < 10 repeat
            matrix[i][j] := 0;
            j := j + 1;
        end while;
        i := i + 1;
        j := 0;
    end while;
end f1.