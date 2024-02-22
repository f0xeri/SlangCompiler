module sample
    public class A inherits Object
        public field-integer i := 10;
        public virtual method foo(A a)()
            output "A::foo called\n";
        end foo;
        public virtual method foo(A a)(in integer x)
            output "A::foo(in integer) called";
        end foo;
        public method bar(A a)()
            output "A::bar called\n";
        end bar;
    end A;
    public class B inherits A
        public virtual method foo(B b)()
            output "B::foo called\n";
        end foo;
        public method bar(B b)()
            output "B::bar called\n";
        end bar;
    end B;
start
    variable-A a;
    variable-B b;
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
        output "\n";
        let i := i + 1;
    end while;
end sample.