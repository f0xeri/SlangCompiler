module funcOverloading
    private function add(in integer a, in integer b): integer
        return a + b;
    end add;

    private function add(in integer a, in integer b, in integer c): integer
        return a + b + c;
    end add;

    private function add(in real a, in real b): real
        return a + b;
    end add;

    private function add(in real a, in real b, in real c): real
        return a + b + c;
    end add;

    private class TestClass inherits Object
        public method add(TestClass this)(in integer a, in integer b): integer
            return a + b;
        end add;

        public method add(TestClass this)(in integer a, in integer b, in integer c): integer
            return a + b + c;
        end add;

        public method add(TestClass this)(in real a, in real b): real
            return a + b;
        end add;

        public method add(TestClass this)(in real a, in real b, in real c): real
            return a + b + c;
        end add;
    end TestClass;
start
    variable-integer a := 1;
    variable-integer b := 2;
    variable-integer c := 3;
    variable-real ar := 1.1;
    variable-real br := 2.2;
    variable-real cr := 3.3;
    output "a + b = ";
    output add(a, b);
    output "\n";
    output "a + b + c = ";
    output add(a, b, c);
    output "\n";
    output "ar + br = ";
    output add(ar, br);
    output "\n";
    output "ar + br + cr = ";
    output add(ar, br, cr);
    output "\n";
    output "a + br = ";
    output add(a, br);
    output "\n";
    output "ar + b = ";
    output add(ar, b);
    output "\n";
    output "ar + b + c = ";
    output add(a, br, cr);
    output "\n";
    output "a + b + cr = ";
    output add(ar, b, c);
    output "\n";

    variable-TestClass tc;
    output "tc.add(a, b) = ";
    output tc.add(a, b);
    output "\n";
    output "tc.add(a, b, c) = ";
    output tc.add(a, b, c);
    output "\n";
    output "tc.add(ar, br) = ";
    output tc.add(ar, br);
    output "\n";
    output "tc.add(ar, br, cr) = ";
    output tc.add(ar, br, cr);
    output "\n";
    output "tc.add(a, br) = ";
    output tc.add(a, br);
    output "\n";
    output "tc.add(ar, b) = ";
    output tc.add(ar, b);
    output "\n";
    output "tc.add(ar, b, c) = ";
    output tc.add(ar, b, c);
    output "\n";
    output "tc.add(a, b, cr) = ";
    output tc.add(a, b, cr);
    output "\n";
end funcOverloading.
