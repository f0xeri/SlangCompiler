import StdString;
module StdStringTests
start
    variable-array[256] character array;
    let array := "chararray";
    output array;
    output StdString.strlen(array);
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
    call str2.init(array);
    output str2.toString();
    output str2.capacity;
    call str2.clear();

    variable-StdString.String str3;
    variable-integer i := 0;
    while i < 100000 repeat
        call str3.init("test");
        let i := i + 1;
        output i;
    end while;
end StdStringTests.