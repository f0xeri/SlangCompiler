module StdString
    public class String inherits Object
        private field-array[256] character arrayOfChars;
        private field-integer length := 256;
        private field-integer capacity := 256;

        public method init(String this)(in array[] character str, in integer capacity)
            variable-array[capacity] character newArrayOfChars;
            let newArrayOfChars := str;
            let this.arrayOfChars := newArrayOfChars;
            let this.capacity := capacity;
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