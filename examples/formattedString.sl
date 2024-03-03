module test
start
    variable-integer i := 5;
    variable-real r := 5.5;
    variable-float f := 6.6f;
    variable-boolean b := true;
    variable-character c := "c";
    variable-array[] character s := "test str";
    output f"i = {i + 1};\nr = {r + 1.1};\nf = {f + 1.1f};\nb = {b};\nc = \"{c}\";\ns = \"{s}\";";
end test.