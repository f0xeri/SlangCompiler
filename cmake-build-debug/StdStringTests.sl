import StdString;
module StdStringTests
    public class ABC inherits Object

    end ABC;
start
    variable-integer i := 0;

    variable-array[10] StdString.String strArr;
    call strArr[0].init("test1");
    call strArr[1].init("test2");
    output strArr[0].toString();
    output strArr[1].toString();
    call strArr[0].concat(strArr[1].toString());
    output strArr[0].toString();
    output strArr[1].toString();
    output strArr[0].length;

    variable-StdString.String str;
    call str.init("test1");
    call str.concat("test2");
    output str.toString();

    variable-StdString.String res;
    let res := str.substr(6, 9);
    while i < 10000000 repeat
        let res := str.substr(6, 9);
        let i := i + 1;
    end while;
    output res.toString();
    let res := str.substr(6);
    output res.toString();
    output str.find("est");
    output str.find("est2");

end StdStringTests.