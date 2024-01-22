import f2;
module funcPointers
    public procedure testProc(out function(in integer, in integer): integer func):
        output func(1, 2);
        output "\n";
    end testProc;

    public function testFunc(in integer x, in integer y): integer
        return 1;
    end testFunc;

    public function testFunc(in integer x): integer
        return 2;
    end testFunc;

    public function testFunc(in real x): integer
        return 3;
    end testFunc;

    public function testFunc(): integer
        return 4;
    end testFunc;

    public function testFunc2(in integer x, in integer y): integer
        return 21;
    end testFunc2;

    public function testFunc2(in integer x): integer
        return 22;
    end testFunc2;

    public function testFunc2(in real x): integer
        return 23;
    end testFunc2;

    public function testFunc2(): integer
        return 24;
    end testFunc2;

    public procedure testProc(in integer x, in integer y):
        output 1;
    end testProc;

    public procedure testProc(in integer x):
        output 2;
    end testProc;

    public procedure testProc(in real x):
        output 3;
    end testProc;

    public procedure testProc():
        output 4;
    end testProc;

    public variable-function(in integer, in integer): integer funcPointer := testFunc;
    //public procedure testProc(in function(in integer, in integer): integer func):
        //output func(1, 2);
    //end testFunc;
start
    output funcPointer(1, 2);
    output "\n";
    variable-function(in integer): integer funcPointer2 := testFunc;
    output funcPointer2(1);
    output "\n";
    variable-function(in real): integer funcPointer3 := testFunc;
    output funcPointer3(1.0);
    output "\n";
    variable-function(): integer funcPointer4 := testFunc;
    output funcPointer4();
    output "\n~~~\n";

    let funcPointer := testFunc2;
    output funcPointer(1, 2);
    output "\n";
    let funcPointer2 := testFunc2;
    output funcPointer2(1);
    output "\n";
    let funcPointer3 := testFunc2;
    output funcPointer3(1.0);
    output "\n";
    let funcPointer4 := testFunc2;
    output funcPointer4();
    output "\n~~~\n";

    variable-procedure(in integer, in integer) procPointer := testProc;
    call procPointer(1, 2);
    output "\n";
    variable-procedure(in integer) procPointer2 := testProc;
    call procPointer2(1);
    output "\n";
    variable-procedure(in real) procPointer3 := testProc;
    call procPointer3(1.0);
    output "\n";
    variable-procedure() procPointer4 := testProc;
    call procPointer4();
    output "\n~~~\n";

    variable-procedure(out function(in integer, in integer): integer) procPointer5 := testProc;
    let funcPointer := testFunc;
    call procPointer5(funcPointer);
    output "~~~\n";
    let funcPointer := testFunc2;
    call procPointer5(funcPointer);

    output "~~~\n";

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //let procPointer5 := testProc;
    //call procPointer5(testFunc);


end funcPointers.