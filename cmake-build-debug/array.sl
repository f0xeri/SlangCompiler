module f1
    variable-array[5] integer globalarr;
start
    let globalarr[0] := 5;
    variable-array[5] integer arr;
    let arr[0] := 1;
    output globalarr[0];
    output arr[0];
end f1.