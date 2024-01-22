module classes2
    public class A2 inherits Object
        public field-integer i := 1;
        public method p1(A2 a2)()
            output "A2 class\n";
        end p1;
    end A2;

    private class B2 inherits Object
        public method p1(B2 b2)()
            output "B2 class\n";
        end p1;
    end B2;

    public variable-integer globalInt := 15;
    private variable-integer globalPrivateInt := 15;

    public procedure foo()()
        output "public foo 2\n";
    end foo;

    private procedure foop()()
        output "private foop 2\n";
    end foop;
start
end classes2.