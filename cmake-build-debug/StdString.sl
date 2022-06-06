module StdString
    public function strlen(in array[] character str): integer
        variable-integer i := 0;
        variable-character ch := str[i];
        while (ch != "\0") repeat
            let i := i + 1;
            let ch := str[i];
        end while;
        return i;
    end strlen;

    public class String inherits Object
        private field-array[256] character arrayOfChars;
        private field-integer length := 256;
        private field-integer capacity := 256;

        public method clear(String this)()
            delete this.arrayOfChars;
            let this.arrayOfChars := nil;
            let this.capacity := 0;
            let this.length := 0;
        end clear;

        public method init(String this)(in array[] character str)
            variable-integer capacity := strlen(str);
            let this.arrayOfChars := str;
            let this.capacity := capacity;
            let this.length := capacity;
        end init;

        public method length(String this)(): integer
            return this.length;
        end length;

        public method toString(String this)(): array[] character
            return this.arrayOfChars;
        end toString;

        public method getSymbol(String this)(in integer pos) : character
            if pos < this.length then
                return this.arrayOfChars[pos];
            else
                output "[RUNTIME ERROR] pos is out of string size.";
            end if;
        end getSymbol;

        public method setSymbol(String this)(in integer pos, in character sym)
            if pos < this.length then
                let this.arrayOfChars[pos] := sym;
            else
                output "[RUNTIME ERROR] pos is out of string size.";
            end if;
        end setSymbol;
    end String;
start
end StdString.