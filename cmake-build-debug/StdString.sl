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

    public function strcpy(in array[] character dest, in array[] character src): array[] character
        variable-integer i := 0;
        variable-character ch := src[i];
        while (ch != "\0") repeat
            let dest[i] := ch;
            let i := i + 1;
            let ch := src[i];
        end while;
        let dest[i] := "\0";
        return dest;
    end strcpy;

    public function strcat(in array[] character dest, in array[] character src): array[] character
        variable-integer i := 0;
        variable-character ch := src[i];
        variable-integer j := strlen(dest);
        while (ch != "\0") repeat
            let dest[j] := ch;
            let i := i + 1;
            let j := j + 1;
            let ch := src[i];
        end while;
        let dest[j] := "\0";
        return dest;
    end strcat;


    public class String inherits Object
        private field-array[256] character arrayOfChars;
        private field-integer length := 256;
        private field-integer capacity := 256;

        public method clear(String this)()
            delete this.arrayOfChars;
            let this.arrayOfChars := "";
            let this.capacity := 0;
            let this.length := 0;
        end clear;

        // TODO: does it leak?
        public method init(String this)(in array[] character str)
            delete this.arrayOfChars;
            variable-integer capacity := strlen(str);

            variable-array[capacity + 1] character newCharArray;
            let newCharArray := strcpy(newCharArray, str);

            let this.arrayOfChars := newCharArray;
            let this.capacity := capacity;
            let this.length := capacity;
        end init;

        //  TODO: does it leak?
        public method concat(String this)(in array[] character str)
            variable-integer strLength := strlen(str);
            variable-integer totalLength := this.length + strLength;

            variable-array[totalLength + 1] character newCharArray;
            let newCharArray := strcpy(newCharArray, this.arrayOfChars);
            let newCharArray := strcat(newCharArray, str);

            delete this.arrayOfChars;
            let this.arrayOfChars := newCharArray;
            let this.capacity := totalLength;
            let this.length := totalLength;
        end concat;

        public method substr(String this)(in integer first, in integer last) : String
            variable-String result;
            if last > this.length then
                call result.init("");
                return result;
            end if;
            variable-integer length := last - first;
            variable-array[length + 1] character newCharArray;

            variable-integer i := first;
            variable-integer j := 0;
            while i < last repeat
                let newCharArray[j] := this.arrayOfChars[i];
                let i := i + 1;
                let j := j + 1;
            end while;
            let newCharArray[j] := "\0";
            call result.init(newCharArray);
            return result;
        end substr;

        // TODO: implement this.method() calls
        public method substr(String this)(in integer first) : String
            // return this.substr(first, this.length);
            variable-integer last := this.length;
            variable-String result;
            if last > this.length then
                call result.init("");
                return result;
            end if;
            variable-integer length := last - first;
            variable-array[length + 1] character newCharArray;

            variable-integer i := first;
            variable-integer j := 0;
            while i < last repeat
                let newCharArray[j] := this.arrayOfChars[i];
                let i := i + 1;
                let j := j + 1;
            end while;
            let newCharArray[j] := "\0";
            call result.init(newCharArray);
            return result;
        end substr;

        public method find(String this)(in character ch) : integer
            variable-integer i := 0;
            while i < this.length repeat
                if this.arrayOfChars[i] == ch then
                    return i;
                end if;
                let i := i + 1;
            end while;
            return -1;
        end find;

        public method find(String this)(in array[] character str) : integer
            variable-integer i := 0;
            variable-integer strLen := strlen(str);
            while i < this.length repeat
                if this.arrayOfChars[i] == str[0] then
                    variable-integer j := 0;
                    variable-boolean failed := false;
                    while j < strLen && failed != true repeat
                        if this.arrayOfChars[i + j] != str[j] then
                            let failed := true;
                        end if;
                        if j == strLen - 1 && failed != true then
                            return i;
                        end if;
                        let j := j + 1;
                    end while;
                end if;
                let i := i + 1;
            end while;
            return -1;
        end find;

        public method len(String this)(): integer
            return this.length;
        end len;

        public method toString(String this)(): array[] character
            return this.arrayOfChars;
        end toString;

        public method getSymbol(String this)(in integer pos) : character
            if pos < this.length then
                return this.arrayOfChars[pos];
            end if;
        end getSymbol;

        public method setSymbol(String this)(in integer pos, in character sym)
            if pos < this.length then
                let this.arrayOfChars[pos] := sym;
            end if;
        end setSymbol;
    end String;
start
end StdString.