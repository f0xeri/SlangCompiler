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
        let func := testFunc;
        output func;
        //output func(1, 2);
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
    //call procPointerOut(testFunc);       // should give error
    call procPointerOut(funcPointer);    // should work but throw error in call         // crashes here because out func() is not loaded inside function
    //call procPointerVar(testFunc);       // should give error         // crashes here because var func() is loaded twice inside function
    call procPointerVar(funcPointer);    // should work
    output funcPointer;
    variable-integer _width := 800;
    variable-integer _height := 600;
    variable-float x :=  _width * 1.0f / _height;
end sample.