module swap
    private procedure test2(var real a, var real b):
        variable-real temp := a;
        let a := b;
        let b := temp;
    end test2;

    private procedure test(var integer a, var integer b):
        variable-integer temp := a;
        let a := b;
        let b := temp;
    end test;
start
    variable-real a := 5.1;
    variable-real b := 10.2;
    call test2(a, b);
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

