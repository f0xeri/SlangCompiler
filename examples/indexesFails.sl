module indexesFails
    public variable-integer globalInt := 1;
    public variable-array[19] integer globalArr;

    public class Type inherits Object
        public field-array[10] integer arr;
        public field-array[2] array[3] array[4] integer arr3d;
        public field-integer int := 1;

        public method test(Type this)(in array[] character str, in array[] array[] integer newStr)
            output str[1];
            output newStr[1][2];
            output this.arr[1];

            //output this.arr[1][1];       // too many indexes
            //output this.int[1];          // not an array
        end test;
    end Type;

    public function strlen(in array[] character str): integer
        variable-integer i := 0;
        variable-character ch := str[i];
        while (ch != "\0") repeat
            let i := i + 1;
            let ch := str[i];
        end while;
        return i;
    end strlen;

start
    variable-integer localInt := 5;
    variable-array[10] integer localArray;

    //output globalInt[1];            // not an array
    output globalArr[20];
    //output globalArr[1][1];         // too many indexes

    //output localInt[1];             // not an array
    output localArray[11];
    //output localArray[1][1];        // too many indexes

    variable-Type localType;
    //output localType.int[1];        // not an array
    output localType.arr[11];
    //output localType.arr[1][1];     // too many indexes
end indexesFails.
