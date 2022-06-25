import StdString;
    // TODO: fix global objects and arrays of objects
module StdStringTests
start
    variable-integer i := 0;

    // arrays of objects are broken
    variable-array[10] StdString.String strArr;
    call strArr[0].init("test1");
    call strArr[1].init("test2");
    output strArr[0].toString();
    output strArr[1].toString();
    call strArr[0].concat(strArr[1].toString());
    output strArr[0].toString();
    output strArr[1].toString();
    output strArr[0].length;
    call strArr[0].init(strArr[1].toString());
    output "!!!";
    output strArr[0].toString();
    output strArr[1].toString();
    call strArr[1].concat(strArr[1].toString());
    output strArr[0].toString();
    output strArr[1].toString();
    output "!!!";
    variable-StdString.String str;
    call str.init("test1");
    call str.concat("test2");
    output str.toString();

    variable-StdString.String res;
    let res := str.substr(6, 9);
    while i < 100 repeat
        let res := str.substr(6, 9);
        let i := i + 1;
    end while;
    output res.toString();
    let res := str.substr(6);
    output res.toString();
    output str.find("est");
    output str.find("est2");

    variable-StdString.String str2;
    call str2.init("te11st11");
    output StdString.strlen(str2.arrayOfChars);
    output str2.len();
    let i := 0;
    while i < 50 repeat
        call str2.concat("test11211");
        call str2.replaceAll("11", "ELEVEN");
        let i := i + 1;
    end while;
    output StdString.strlen(str2.arrayOfChars);
    output str2.len();
    output str2.toString();

    // create tests for StdString.String.insert()
    variable-StdString.String str3;
    call str3.init("test11string11test23str90111");
    output str3.toString();
    call str3.insert(0, "0");
    call str3.insert(3, "3");
    output str3.toString();
    output "done";
end StdStringTests.