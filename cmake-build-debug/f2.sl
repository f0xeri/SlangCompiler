import f3;
module f2
    public variable-integer i := 1;
    public function func2() : integer
        return f3.func3() + f3.v3 + i;
    end func2;
    public function func2_2() : integer
        return -(f3.func3() + f3.v3 + i);
    end func2_2;
start
end f2.