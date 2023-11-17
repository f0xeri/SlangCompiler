module sample
    private extern function system(in array[] character x): integer
    end system;

    private function getFloat(): float
        return 3.14f;
    end getFloat;

    private function getFloat2(): float
        return getFloat();
    end getFloat2;
    public variable-integer globalInt := 10;
    private class A inherits Object
        public field-array[11] character str := "test";
        public field-array[2] array[3] array[4] integer arr3d;
        public field-integer i := 1;
        private field-integer ip := 11;
        public method p1(A a)(in integer i2, in float f): integer
            output "A class\ni2 = ";
            output i2;
            output ", f = ";
            output f;
            output "\n\n";
            return i2;
        end p1;
        public method getFloat(A a)(): float
            return 1.2f;
        end getFloat;

        public method getA(A a)(): A
            return a.getAp();
        end getA;

        private method getAp(A a)(): A
            return a;
        end getAp;
    end A;

    private function getA(): A
        variable-A a;
        return a;
    end getA;

    private procedure printHelloProc(in array[] array[] character word)
        output "Hello, ";
	    output word;
    end printHelloProc;

    private function printHelloFunc(in array[] array[] character word): array[] array[] character
        output "Hello, ";
	    output word;
	    return word;
    end printHelloFunc;

    public procedure testProc(in integer x, in integer y)
        output 1;
    end testProc;

    public procedure testProc(out function(in integer, in integer): integer func)
        output func(1, 2);
        output "\n";
    end testProc;

    public procedure testProc(in integer x)
        output 2;
    end testProc;

    public procedure testProc(in real x)
        output 3;
    end testProc;

    public procedure testProc()
        output 4;
    end testProc;

    private class Animal inherits Object
        private field-integer i := 1;
        private method getFloat(Animal a)(): float
            return 1.2f;
        end getFloat;
    end Animal;

    private class Dog inherits Animal
        public method getA(Dog a)()
            variable-Animal a2;
            output a2.i;
            output a2.getFloat();
        end getA;
    end Dog;

    private class Shepherd inherits Dog
        public method getA(Shepherd a)()
            variable-Animal a2;
            output a2.i;
            output a2.getFloat();
        end getA;
    end Shepherd;

start
    output globalInt;
    // test line comment
    variable-A a;
    variable-integer ai2 := getA().getA().getA().arr3d[1][2][3];
    variable-integer i := 1;
    variable-procedure(in integer, in integer) procPointer := testProc;
    variable-procedure(in integer) procPointer2 := testProc;
    variable-procedure(in real) procPointer3 := testProc;
    variable-procedure() procPointer4 := testProc;
    variable-procedure(out function(in integer, in integer): integer) procPointer5 := testProc;
    variable-array[2] array[3] array[4] procedure(in integer, in integer) arr3d2;
    let arr3d2[1][2][3] := testProc;
    output arr3d2[1][2][4](1, 2);
    //while xdd == ad(aaa).a[5] && gg != 5 repeat
        //call printHello("aaa");
        //variable-array[2] array[3] array[4] integer arr3d2;
        //variable-array[xdd] xdd2.aa arr3d2;
        //let arr3d2[xdd().xdd()][2][3] := 1;
        //variable-integer i := arr[xdd](22);
    //end while;
end sample.