module main
start
    public variable-integer n;
    input n;

    public variable-integer result := 1;
    if n < 0 then
        let result := 0;
    else if n == 0 then
        let result := 1;
    else
        variable-integer i := 1;
        while i <= n repeat
            let result := result * i;
            let i := i + 1;
        end while;
    end if;

    output "${n}! = ${result}";
end main.