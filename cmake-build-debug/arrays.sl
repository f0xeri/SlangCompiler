import StdString;
module arrays
start
    variable-array[20] array[30] array[40] integer arr3d;

    variable-integer i := 0;
    variable-integer j := 0;
    variable-integer k := 0;
    variable-integer counter := 1;

    while i < 20 repeat
        while j < 30 repeat
            while k < 40 repeat
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
    while i < 20 repeat
        while j < 30 repeat
            while k < 40 repeat
                output arr3d[i][j][k];
                let k := k + 1;
            end while;
            let k := 0;
            let j := j + 1;
        end while;
        let j := 0;
        let i := i + 1;
    end while;
end arrays.