module main
    variable-integer globalint := 15;
    public class Time inherits Object

        public field-integer int := 2 + (1 + 4) * 2;
        public field-real r := 15.25;

        public field-character ch := "!";

        public field-integer int1;
        public field-real r1;

        public field-character ch1;

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

    public function first(in integer a, in integer ra):integer
        let ra := (globalint + (ra - a) + (1 + 4) * 2) * -10;
        return ra;
    end first;

start
    variable-character c := "c";
    variable-Time time;
    variable-integer refarg := globalint + 10;
    variable-integer fcall := first(100, refarg);
    output first(100, 90);
    output refarg;
    let refarg := 10;

    variable-integer n = 0;
    variable-integer result := 1;
    if n < 0 then
        let result := 0;
        if result == 0 then
          output 999;
        end if;
        output result;
    elseif n == 0 then
        let result := 1;
        output result;
    else
        let result := -1;
        output result;
    end if;
    output refarg + 5;
end main.