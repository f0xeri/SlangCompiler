module StdString
    public class String inherits Object
        private field-array[256] character arrayOfChars;
        private field-integer i := 15;
        public method toString(String this)(): array[] character
            return this.arrayOfChars;
        end toString;
    end String;
start
end StdString.