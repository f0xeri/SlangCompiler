module sample
    public function testFunc(in integer x, in integer y): integer
        return x + y;
    end testFunc;

    public function testFunc2(in integer x, in integer y): integer
        return x - y;
    end testFunc2;

    public procedure testProc(in integer x, in integer y)
        output "\ntestProc(in integer x, in integer y):";
        output x + y;
    end testProc;

    public procedure testProc(out function(in integer, in integer): integer func)
        output "\ntestProc(out function(in integer, in integer): integer func)";
        output func(1, 2);
    end testProc;

    public procedure testProc(in integer x)
        output "\ntestProc(in integer x)";
        output x;
    end testProc;

    public procedure testProc(in real x)
        output "\ntestProc(in real x)";
        output x;
    end testProc;

    public procedure testProc()
        output "\ntestProc()";
    end testProc;

    public procedure testInOutVar(in integer x, out integer y, var integer z, out integer y2)
        output "\ntestInOutVar(in integer x, out integer y, var integer z)";
        output x;
        output y;
        output z;
        variable-integer x1 := x;
        variable-integer y1 := y;    // this should throw an error in check (as well as let y1 := y)
        variable-integer z1 := z;
        output "";
        output x1;
        output y1;
        output z1;
        let x := z;
        let y := -1;
        let z := 88;
        output y;
    end testInOutVar;

    public procedure testout(in integer x, out integer y)
        output "\ntestout(in integer x, out integer y)";
        variable-integer x2 := x;
        variable-integer y2 := y;    // this should throw an error in check (as well as let y1 := y)
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
    output "after call testInOutVar";
    output x;
    output y;
    output z;
    call testout(x, y);

    output "\ncheck swaps";
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

    output "\ncheck array of func pointers";
    variable-array[2] function(in integer, in integer): integer funcArray;
    let funcArray[0] := testFunc;
    let funcArray[1] := testFunc2;
    output funcArray[0](1, 2);
    output funcArray[1](1, 2);
end sample.