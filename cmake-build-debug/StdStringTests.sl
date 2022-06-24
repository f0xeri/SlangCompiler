import StdString;
module StdStringTests
start
    variable-integer i := 0;

    variable-StdString.String str2;
    call str2.init("te11st11");
    output StdString.strlen(str2.arrayOfChars);
    output str2.len();
    let i := 0;
    while i < 50000 repeat
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