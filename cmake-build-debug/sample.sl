module sample
    public class A inherits Object
        public field-integer i := 10;
        public field-real r := 11.02;
        public virtual method foo(A a)()
            output "A::foo called";
        end foo;
        public virtual method foo(A a)(in integer x)
            output "A::foo(in integer) called";
        end foo;
        public method bar(A a)()
            output "A::bar called";
        end bar;
    end A;
    public class B inherits A
        public field-integer i2 := 20;
        public virtual method foo(B b)()
            output "B::foo called";
        end foo;
    end B;
start
    variable-A a;
    variable-B b;
    let b.i2 := 30;
    let b.i := 40;
    let b.r := 50.0;
    output b.i2;
    output b.i;
    output b.r;
    variable-A a2;
    variable-B b2;
    variable-array[5] A arr;
    let arr[0] := a;
    let arr[1] := b;
    let arr[2] := b2;
    let arr[3] := a2;
    let arr[4] := a;
    variable-integer i := 0;
    while i < 5 repeat
        call arr[i].foo();
        call arr[i].bar();
        let arr[i].i := i;
        output arr[i].i;
        let i := i + 1;
    end while;
end sample.