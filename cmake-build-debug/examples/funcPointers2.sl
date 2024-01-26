module sample
    public function testFunc(in integer x, in integer y): integer
        return x + y;
    end testFunc;

    public function testFunc2(in integer x, in integer y): integer
        return x - y;
    end testFunc2;

    public procedure testProc(in function(in integer, in integer): integer func)
        output "\ntestProc(in function(in integer, in integer): integer func)";
        output func(1, 2);
        output func;
        let func := testFunc2;
        output func;
        output func(1, 2);

    end testProc;
    public procedure testProc(out function(in integer, in integer): integer func)
        output "\ntestProc(out function(in integer, in integer): integer func)";
        output func;
        // output func(1, 2);    // should give error: Cannot call function using function pointer 'func' passed as 'out' parameter.
        let func := testFunc;    // should not work, but does (casts (int(*)(int, int) to (int(**)(int, int))
        output func;
        // output func(1, 2);    // should give error: Cannot call function using function pointer 'func' passed as 'out' parameter.
    end testProc;
    public procedure testProc(var function(in integer, in integer): integer func)
        output "\ntestProc(var function(in integer, in integer): integer func)";
        output func(1, 2);
        output func;
        let func := testFunc2;
        output func;
        output func(1, 2);
    end testProc;
    public class testClass inherits Object
        public field-integer x := 99;
    end testClass;
start
    variable-function(in integer, in integer): integer funcPointer := testFunc;
    variable-procedure(in function(in integer, in integer): integer) procPointerIn := testProc;
    variable-procedure(out function(in integer, in integer): integer) procPointerOut := testProc;
    variable-procedure(var function(in integer, in integer): integer) procPointerVar := testProc;
    call procPointerIn(testFunc);        // should work
    call procPointerIn(funcPointer);     // should work
    output funcPointer;
    // call procPointerOut(testFunc);    // should give error: No matching function for call (can't pass function as 'out' parameter)
    call procPointerOut(funcPointer);    // should work but throw error in call inside
    // call procPointerVar(testFunc);    // should give error: No matching function for call (can't pass function as 'var' parameter)
    call procPointerVar(funcPointer);    // should work
    output funcPointer;
end sample.