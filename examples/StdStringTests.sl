import StdString;
    // TODO: fix global objects and arrays of objects
    //public variable-array[10] StdString.String globalStrArr;  // should be compile error
module StdStringTests
    public variable-StdString.String globalStr;
    public variable-array[10] StdString.String globalStrArr;
start
    variable-integer i := 0;

    variable-array[10] StdString.String strArr;
    call strArr[0].init("test1");
    call strArr[1].init("test2");
    output strArr[0].toString();
    output "\n";
    output strArr[1].toString();

    output "\n~1~\n";
    call strArr[0].concat(strArr[1].toString());
    output strArr[0].toString();
    output "\n";
    output strArr[1].toString();
    output "\n";
    //output strArr[0].length;        // should be compile error because length is private

    output "\n~2~\n";
    call strArr[0].init(strArr[1].toString());
    output strArr[0].toString();
    output "\n";
    output strArr[1].toString();

    output "\n~3~\n";
    call strArr[1].concat(strArr[1].toString());
    output strArr[0].toString();
    output "\n";
    output strArr[1].toString();

    output "\n~4~\n";
    variable-StdString.String str;
    call str.init("test1");
    call str.concat("test2");
    output str.toString();
    //output str.length;              // should be compile error because length is private

    output "\n~5~\n";
    variable-StdString.String res;
    let res := str.substr(6, 9);
    output res.toString();
    output "\n";
    while i < 100 repeat
        let res := str.substr(6, 9);
        let i := i + 1;
    end while;
    output res.toString();
    output "\n";
    let res := str.substr(6);
    output res.toString();
    output "\n";
    output str.find("est");
    output "\n";
    output str.find("est2");

    output "\n~6~\n";
    variable-StdString.String str2;
    call str2.init("te11st11");
    //output str2.arrayOfChars;                               // should be compile error because length is private
    //output StdString.strlen(str2.arrayOfChars);             // should be compile error because arrayOfChars is private
    output str2.len();
    output "\n";
    let i := 0;
    while i < 10 repeat
        call str2.concat("test11211");
        call str2.replaceAll("11", "ELEVEN");
        let i := i + 1;
    end while;
    //output StdString.strlen(str2.arrayOfChars);             // should be compile error because arrayOfChars is private
    output str2.len();
    output "\n";
    output str2.toString();

    output "\n~7~\n";
    // create tests for StdString.String.insert()
    variable-StdString.String str3;
    call str3.init("test11string11test23str90111");
    output str3.toString();
    output "\n";
    call str3.insert(0, "0");
    call str3.insert(3, "3");
    output str3.toString();

    output "\n~8~\n";
    call globalStr.init("global test1");
    output globalStr.toString();
    output "\n";
    call globalStr.concat(" new text");
    output globalStr.toString();
    //output globalStr.length;                // should be compile error because length is private
    //output globalStr.arrayOfChars;          // should be compile error because arrayOfChars is private
    output "\n";
    call globalStrArr[0].init("global arr0");
    call globalStrArr[1].init("global arr1");
    output globalStrArr[0].toString();
    output "\n";
    output globalStrArr[1].toString();
    //output globalStrArr[0].length;         // should be compile error because length is private
    //output globalStrArr[0].arrayOfChars;   // should be compile error because arrayOfChars is private
    output "\n\ndone\n";
end StdStringTests.
