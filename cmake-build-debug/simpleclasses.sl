module simpleclasses
    public variable-integer globalint := 15;
    public class Time inherits Object
        public field-integer int := 2;
        public field-real r := 15.25;

        public field-character ch := "!";

        public field-integer int1;
        public field-real r1;

        public field-character ch1;

        public method p1(Time time)()
            output "parent method";
        end p1;
    end Time;

    public variable-real globalreal := 100.1;

    public class ABC inherits Time

    end ABC;

    public procedure foo(in integer a, out Time time)()
        output a;
        output time.int;
    end foo;

    public function first(in integer a, in integer ra): integer
        let ra := (globalint + (ra - a) + (1 + 4) * 2) * -10;
        return ra;
    end first;

    public function factorial(in integer n): integer
        variable-integer result := 1;
            if n < 0 then
                let result := 0;
            else if n == 0 then
                let result := 1;
            else
                variable-integer i := 1;
                while i <= n repeat
                    let result := result * i;
                    let i := i + 1;
                end while;
            end if;
            end if;
        return result;
    end factorial;

    public function factorialRecursive(in integer n): integer
        if n == 1 then
    	    return 1;
        else
    	    return n * factorialRecursive(n - 1);
    	end if;
    end factorialRecursive;

    public function fibRecursive(in integer n): integer
        if n <= 1 then
            return n;
        else
            return fibRecursive(n - 1) + fibRecursive(n - 2);
        end if;
    end fibRecursive;

    public procedure printFibNumbers(in integer maxN)
        variable-integer i := 0;
        while i <= maxN repeat
            output fibRecursive(i);
            let i := i + 1;
        end while;
    end printFibNumbers;
start
    output factorial(5);
    output factorialRecursive(5);
    call printFibNumbers(10);
    output globalreal + globalint;
    output globalint + globalreal;
    variable-real aaa := globalint / 2.0;
    output aaa;
    output "~~~~~~~~~~~~~";
    variable-Time time;
    output time.int;
    output time.r;
    output "~~~~~~~~~~~~~";
    variable-ABC abc;
    call foo(5, time);
end simpleclasses.