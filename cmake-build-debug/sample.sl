module swap
    private procedure test(var integer a, var integer b)
        variable-integer temp := a;
        let a := b;
        let b := temp;
        output a;
        output b;
    end test;

    public procedure testFunc(out integer x, out integer y)
        output x;
        output y;
        variable-integer z := x;
        output z;
        let x := 100;
        output x;
        let x := z;
        output x;
        let x := y;
        output x;
    end testFunc;
start
    variable-integer c := 5;
    variable-integer d := 10;
    call test(c, d);
    output "\n";
    output c;
    output "\n";
    output d;
    call testFunc(c, d);
end swap.

