module main
    variable-string globalstr := "global string";
    variable-integer globalint := 15;
    public class Time inherits Object
        public field-boolean b := 1 > 5;
        public field-string s := "test string!" + "aaa";
        public field-integer int := 2 + (1 + 4) * 2;
        public field-real r := 15.25;

        public field-character ch := "!";

        public field-string s1;
        public field-integer int1;
        public field-real r1;
        public field-boolean b1;
        public field-character ch1;

        public field-Time time;

        public field-array[10] integer arr;
        public field-array[3] array[5] integer arr;

        public method p1(Time time)()
            output "parent method";
        end p1;

        public method p2(Time this)():string
            return this.s;
        end p2;
    end Time;
    variable-real globalreal := 100.1;
    public class ABC inherits Time

    end ABC;

    public function first(in string a):string
        return 1 == 2;
    end first;

    private procedure second(in string a, var integer b)

    end second;

start
    variable-ABC abc;
    variable-boolean a := 5.3 <= 3;
    variable-boolean b := false;
    variable-character c := "!";
    output "p2 -> ${abc.p2()} \n";

    if (5 > 3 && 5 > 2) then
        output "test";
    end if;
end main.