import StdString;
module StdStringTests
start
    variable-integer i := 0;
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