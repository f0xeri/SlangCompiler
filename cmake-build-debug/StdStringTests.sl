import StdString;
module StdStringTests
start
    variable-StdString.String str;
    let str.arrayOfChars[0] := "s";
    let str.arrayOfChars[1] := "t";
    let str.arrayOfChars[2] := "r";
    let str.arrayOfChars[3] := "i";
    let str.arrayOfChars[4] := "n";
    let str.arrayOfChars[5] := "g";
    output str.toString();
end StdStringTests.