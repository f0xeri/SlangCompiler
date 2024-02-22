module arrays
    public class A inherits Object
        public field-integer bbb := 1010;
        public field-array[10] character str := "govno";
    end A;

    public variable-array[2] array[3] array[4] integer arr3d;
    public variable-integer a := 0;
    public variable-A a1;
start
    variable-integer i := 0;
    variable-integer j := 0;
    variable-integer k := 0;
    variable-integer counter := 1;
    // output i.bbbb; // should be error, fix it!
    output a1.bbb;
    output "\n";
    output a1.str[0];
    output "\n";
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

    // create 4x4 matrix
    variable-array[4] array[4] integer matrix;
    let i := 0;
    let j := 0;
    // fill it as a rotation matrix
    while i < 4 repeat
        while j < 4 repeat
            if i == j then
                let matrix[i][j] := 1;
            else
                let matrix[i][j] := 0;
            end if;
            let j := j + 1;
        end while;
        let j := 0;
        let i := i + 1;
    end while;

    // pretty print the matrix
    let i := 0;
    let j := 0;
    while i < 4 repeat
        while j < 4 repeat
            output matrix[i][j];
            let j := j + 1;
        end while;
        let j := 0;
        let i := i + 1;
        output "\n";
    end while;
end arrays.

