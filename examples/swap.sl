module swap
    private procedure swapReal(var real a, var real b):
        variable-real temp := a;
        let a := b;
        let b := temp;
    end swapReal;

    private procedure test(var integer a, var integer b):
        variable-integer temp := a;
        let a := b;
        let b := temp;
    end test;
start
    variable-real a := 5.1;
    variable-real b := 10.2;
    call swapReal(a, b);
    output a;
    output "\n";
    output b;

    variable-integer c := 5;
    variable-integer d := 10;
    call test(c, d);
    output "\n";
    output c;
    output "\n";
    output d;
end swap.

