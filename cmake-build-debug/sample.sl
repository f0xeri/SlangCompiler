module main
start
    // test comment
    variable-integer i := 2 + (1 + 4) * 2;      // test trailing comment
    variable-real r := 3.1415 * 2;
    variable-float f := -3.14f;
    variable-boolean b := true && false;
    variable-boolean b2 := 1 != 2;
    output i;
    output "line\n";
    output r;
end main.