import classes2;
module main
    public class A inherits Object
        public field-array[11] character str := "public A string";
        public field-integer i := 1;
        private field-integer ip := 11;
        public method p1(A a)()
            output "A class\n";
        end p1;
    end A;

    private class B inherits A
        public method p1(B b)()
            output "B class\n";
        end p1;

        private method p2(B b)()
            output "private B class\n";
        end p2;
    end B;
start
    variable-A a;
    call a.p1();
    output a.i;
    //output a.ip;


    variable-B b;
    call b.p1();
    output b.i;
    //call b.p2();

    variable-classes2.A2 a2;
    call a2.p1();
    output a2.i;

    //variable-classes2.B2 b2;
    //call b2.p1();

    call classes2.foo();
    //call classes2.foop();

    output classes2.globalInt;
    //output classes2.globalPrivateInt;
end main.