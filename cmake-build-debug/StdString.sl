module StdString
    public class String inherits Object
        private field-array[256] character arrayOfChars;
        private field-integer size := 256;

        public method length(String this)(): integer
            return this.size;
        end length;

        public method toString(String this)(): array[] character
            return this.arrayOfChars;
        end toString;

        public method getSymbol(String this)(in integer pos) : character
            if pos < this.size then
                return this.arrayOfChars[pos];
            else
                output "[RUNTIME ERROR] pos is out of string size.";
            end if;

        end getSymbol;

        public method setSymbol(String this)(in integer pos, in character sym)
            if pos < this.size then
                let this.arrayOfChars[pos] := sym;
            else
                output "[RUNTIME ERROR] pos is out of string size.";
            end if;
        end setSymbol;
    end String;
start
end StdString.