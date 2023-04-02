module arrays
start
    variable-array[2] array[3] array[4] integer arr3d;

    variable-integer i := 0;
    variable-integer j := 0;
    variable-integer k := 0;
    variable-integer counter := 1;

    while i < 2 repeat
        while j < 3 repeat
            while k < 4 repeat
                let arr3d[i][j][k] := counter;
                let counter := counter + 1;
                let k := k + 1;
            end while;
            let k := 0;
            let j := j + 1;
        end while;
        let j := 0;
        let i := i + 1;
    end while;

    let i := 0;
    let j := 0;
    let k := 0;
    while i < 2 repeat
        while j < 3 repeat
            while k < 4 repeat
                output arr3d[i][j][k];
                let k := k + 1;
            end while;
            let k := 0;
            let j := j + 1;
            output "\n";
        end while;
        let j := 0;
        let i := i + 1;
        output "\n";
    end while;
end arrays.