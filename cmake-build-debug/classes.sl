import classes2;
module classes
    public variable-integer globalInt := 1;
    private variable-float globalFloat := 99.9f;
    public class A inherits Object
        // something goes wrong with debug info, when we have an array as field
        public field-array[11] character str := "test"; // inline init for array doesn't work
        public field-array[2] array[3] array[4] integer arr3d;
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
        public field-float ff := 2.22f;
        public field-array[11] character str2 := "test2";
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
    //variable-integer i := 0;
    variable-A a;
    call a.p1(1, 1.1f);
    output a.i;
    let a.str := "test";
    //output a.ip;

    variable-B b;
    call b.p1(2, 2.2f);
    output b.i;
    output b.ff;
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
    let a4.i := 444;
    output "~~~~\n";
    call a4.p1(3, 3.3f);
    output a4.i;

    //let i := printHello("test");
    //output i;
    output "\n";
    variable-integer i := 0;
    variable-integer j := 0;
    variable-integer k := 0;
    variable-integer counter := 1;

    while i < 2 repeat
        while j < 3 repeat
            while k < 4 repeat
                let a.arr3d[i][j][k] := counter;
                let counter := counter + 1;
                let k := k + 1;
            end while;
            let k := 0;
            let j := j + 1;
        end while;
        let j := 0;
        let i := i + 1;
    end while;

    let i := 0;
    let j := 0;
    let k := 0;
    while i < 2 repeat
        while j < 3 repeat
            while k < 4 repeat
                output a.arr3d[i][j][k];
                let k := k + 1;
            end while;
            let k := 0;
            let j := j + 1;
            output "\n";
        end while;
        let j := 0;
        let i := i + 1;
        output "\n";
    end while;

end classes.