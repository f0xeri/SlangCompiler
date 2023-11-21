module sample
start
    variable-integer x := 0;
    variable-array[2] array[4] character arr;
    let arr[x][x] := "x";
    let arr[x][x + 1] := "y";
    let arr[x][x + 2] := "z";
    let arr[x][x + 3] := "\0";

    output arr[0];
    let arr[x + 1] := arr[x];
    output arr[1];

    variable-array[] character str := arr[x];
    output str;

    let arr[x][x] := "z";
    let arr[x][x + 1] := "y";
    let arr[x][x + 2] := "x";
    let arr[x][x + 3] := "\0";
    output arr[0];
    output arr[1];
    output str;
end sample.