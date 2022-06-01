import StdString;
module StdStringTests
start
    variable-array[256] character array;
    let array := "chararray";
    output array;
    variable-StdString.String str;
    let str.arrayOfChars[0] := "s";
    let str.arrayOfChars[1] := "t";
    let str.arrayOfChars[2] := "r";
    let str.arrayOfChars[3] := "i";
    let str.arrayOfChars[4] := "n";
    let str.arrayOfChars[5] := "g";
    call str.setSymbol(0, "a");
    output str.toString();

    variable-StdString.String str2;
    call str2.init("govno", 228);
    output str2.toString();
    output str2.capacity;
end StdStringTests.