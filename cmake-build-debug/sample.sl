module sample
    public class A inherits Object
        public field-integer i := 10;
        public virtual method foo(A a)()
            output "A::foo called";
        end foo;
        public method bar(A a)()
            output "A::bar called";
        end bar;
    end A;
    public class B inherits A
        public virtual method foo(B b)()
            output "B::foo called";
        end foo;
        public method bar(B a)()
            output "B::bar called";
        end bar;
    end B;
start
    variable-A a;
    variable-B b;
    variable-A a2;
    variable-B b2;
    variable-array[5] A arr;
    let arr[0] := a;
    let arr[1] := a;
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
    variable-array[10] integer arr2;
    let arr2[0] := 1;
    let arr2[1] := 2;
    let arr2[2] := 3;
end sample.