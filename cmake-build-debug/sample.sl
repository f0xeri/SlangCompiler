module sample
    public function testFunc(in integer x, in integer y): integer
        return x + y;
    end testFunc;

    public procedure testProc(in integer x, in integer y)
        output "testProc(in integer x, in integer y):";
        output x + y;
    end testProc;

    public procedure testProc(out function(in integer, in integer): integer func)
        output "testProc(out function(in integer, in integer): integer func)";
        output func(1, 2);
    end testProc;

    public procedure testProc(in integer x)
        output "testProc(in integer x)";
        output x;
    end testProc;

    public procedure testProc(in real x)
        output "testProc(in real x)";
        output x;
    end testProc;

    public procedure testProc()
        output "testProc()";
    end testProc;

    public procedure testInOutVar(in integer x, out integer y, var integer z, out integer z2)
        output "testInOutVar(in integer x, out integer y, var integer z)";
        output x;
        output y;
        output z;
        variable-integer x1 := x;
        // variable-integer y1 := y; // this should throw an error in check
        variable-integer y1 := y;
        // let y1 := y;              // this should throw an error in check
        variable-integer z1 := z;
        output "";
        output x1;
        output y1;
        output z1;
    end testInOutVar;

    public procedure testout(in integer x, out integer y)
        output "testout(in integer x, out integer y)";
        variable-integer x2 := x;
        variable-integer y2 := y;
        output x2;
        output y2;
    end testout;

    private procedure swapReal(var real a, var real b)
        variable-real temp := a;
        let a := b;
        let b := temp;
    end swapReal;

    private procedure test(var integer a, var integer b)
        variable-integer temp := a;
        let a := b;
        let b := temp;
    end test;
start
    variable-procedure(in integer, in integer) procPointer := testProc;
    variable-procedure(in integer) procPointer2 := testProc;
    variable-procedure(in real) procPointer3 := testProc;
    variable-procedure() procPointer4 := testProc;
    variable-procedure(out function(in integer, in integer): integer) procPointer5 := testProc;
    call procPointer(1, 2);
    call procPointer2(1);
    call procPointer3(1.0);
    call procPointer4();
    call procPointer5(testFunc);

    variable-integer x := 1;
    variable-integer y := 2;
    variable-integer z := 3;
    call testInOutVar(x, y, z, z);
    call testout(x, y);

    output "check swaps";
    variable-real a := 5.1;
    variable-real b := 10.2;
    call swapReal(a, b);
    output a;
    output b;

    variable-integer c := 5;
    variable-integer d := 10;
    call test(c, d);
    output c;
    output d;
end sample.