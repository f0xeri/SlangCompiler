import f2;
module f1
    variable-integer i := 1111;
start
    output f2.func2();
    output f2.i;

    variable-integer j := 0;
    while j < 1 repeat
        variable-integer i := -1;
        output i;
        let j := j + 1;
    end while;
    output i;
end f1.