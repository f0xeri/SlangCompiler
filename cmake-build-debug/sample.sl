module sample
    public class A inherits Object
        public field-array[1] procedure() arr;
        public field-integer i := 1;
        private field-integer ip := 11;
        public method p1(A a)(in integer i2, in float f)
            output i2;
            output f;
            output a.i;
            output a.ip;
        end p1;
        public method getA(A a)(): A
            return a;
        end getA;
        public method printSomething(A a)()
            output "Something\n";
        end printSomething;

        public method getMethod(A a)(): procedure(out A, in integer, in float)
            call a.printSomething();
            return a.p1;
        end getMethod;
    end A;

    public function getA(): A
        variable-A a;
        return a;
    end getA;

    public procedure printHello()
        output "Hello World!\n";
    end printHello;
start
    variable-function(): A getAFuncPtr := getA;
    call getAFuncPtr().getA().getA().getA().getA().getA().p1(1, 2.0);
    variable-A a;
    call a.p1(2, 3.0);
    variable-A a2 := getA();
    let a2.arr[0] := printHello;
    call a2.arr[0]();
    call a2.getMethod()(a2, 3, 4.0);
end sample.
