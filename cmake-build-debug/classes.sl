import classes2;
module classes
    public class A inherits Object
        // something goes wrong with debug info, when we have an array as field
        //public field-array[11] character str := "public A string";
        public field-integer i := 1;
        private field-integer ip := 11;
        public method p1(A a)(in integer i2, in float f)
            output "A class\n";
            output i2;
            output f;
            output "\n\n";
        end p1;
    end A;

    private class B inherits A
        public method p1(B b)(in integer i2, in float f)
            output "B class\n";
            output i2;
            output f;
            output "\n\n";
        end p1;

        private method p2(B b)()
            output "private B class\n";
        end p2;
    end B;

    private class C inherits Object
        public field-A aobj;
    end C;

    private function printHello(in array[] character word): integer
        output "Hello, ";
	    output word;
	    return 111;
    end printHello;

start
    variable-integer i := 0;
    variable-A a;
    call a.p1(1, 1.1f);
    output a.i;
    //output a.ip;

    variable-B b;
    call b.p1(2, 2.2f);
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

    variable-C c;
    variable-A a4 := c.aobj;
    output "~~~~\n";
    call a4.p1(3, 3.3f);
    output a4.i;

    let i := printHello("test");
    output i;

end classes.