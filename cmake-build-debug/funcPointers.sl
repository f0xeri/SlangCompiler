import f2;
module funcPointers
    public procedure testProc(out function(in integer, in integer): integer func):
        output func(1, 2);
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
    variable-function(in integer): integer funcPointer2;
    variable-function(in real): integer funcPointer3;
    variable-function(): integer funcPointer4;
    output "~~~";
    variable-procedure(out function(in integer, in integer): integer) procPointer5 := testProc;
    let funcPointer := testFunc;
    call procPointer5(funcPointer);
    let funcPointer := testFunc2;
    // TODO: fix bug - CallExprNode codegen can't find funcPointer declaration
    call procPointer5(funcPointer);
end funcPointers.