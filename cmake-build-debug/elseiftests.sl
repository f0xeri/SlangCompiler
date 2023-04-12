module elseiftests
    private function test(in integer i): integer
        output "\ntest";
        if (i == 0) then
            return 0;
        elseif (i == 1) then
            return 1;
        elseif (i == 2) then
            output "2";
        elseif (i == 3) then
            return 3;
        elseif (i == 4) then
            return 4;
        else
            output "else";
            return 555;
        end if;
        return 999;
    end test;
start
    variable-integer i := 55;
    if (i == 0) then
        output "i == 0\n";
    elseif (i == 1) then
        output "i == 1\n";
    elseif (i == 2) then
        output "i == 2\n";
    elseif (i == 3) then
        output "i == 3\n";
    elseif (i == 4) then
        output "i == 4\n";
    else
        output "i == unknown\n";
    end if;

    output test(0);
    output test(1);
    output test(2);
    output test(3);
    output test(4);
    output test(5);

    output "\n\n";

    let i := 3;
    if (i == 0) then
        output "i == 0\n";
    elseif (i == 1) then
        variable-integer i1 := 1;
        output i1;
    elseif (i == 2) then
        variable-integer i2 := 2;
        output i2;
    elseif (i == 3) then
        variable-integer i3 := 3;
        output i3;
    elseif (i == 4) then
        variable-integer i4 := 4;
        output i4;
    else
        output "i == unknown\n";
    end if;

    while i < 10 repeat
        output i;
        let i := i + 1;
        variable-integer j := i;
        output j;
    end while;
end elseiftests.

