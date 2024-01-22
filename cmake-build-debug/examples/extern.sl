module Extern
    public extern function sin(in real x): real
    end sin;
    public extern function cos(in real x): real
    end cos;
start
    variable-real x := 5.0;
    variable-real y := sin(x);
    variable-real z := cos(x);
    output y;
    output z;
end Extern.