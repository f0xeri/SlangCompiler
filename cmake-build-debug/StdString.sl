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

    // create strstr that returns the index of the first occurrence of the substring in the string
    public function strstr(in array[] character str, in array[] character substr): integer
        variable-integer i := 0;
        variable-integer j := 0;
        variable-character ch := str[i];
        while (ch != "\0") repeat
            let ch := str[i];
            let i := i + 1;
            if (ch == substr[j]) then
                let j := j + 1;
                if (substr[j] == "\0") then
                    return i - j;
                end if;
            else
                let j := 0;
            end if;
        end while;
        return -1;
    end strstr;

    // create strstr that returns index and starts from index
    public function strstr(in array[] character str, in array[] character substr, in integer index): integer
        variable-integer i := index;
        variable-integer j := 0;
        variable-character ch := str[i];
        while (ch != "\0") repeat
            let ch := str[i];
            let i := i + 1;
            if (ch == substr[j]) then
                let j := j + 1;
                if (substr[j] == "\0") then
                    return i - j;
                end if;
            else
                let j := 0;
            end if;
        end while;
        return -1;
    end strstr;


    public class String inherits Object
        private field-array[0] character arrayOfChars;
        private field-integer length := 0;

        public method clear(String this)()
            delete this.arrayOfChars;
            let this.arrayOfChars := "";
            //let this.capacity := 0;
            let this.length := 0;
        end clear;

        // TODO: does it leak?
        public method init(String this)(in array[] character str)
            // delete clears not malloced pointer if we are passing element of array of objects
            // TODO: fix array of objects initialization
            //delete this.arrayOfChars;
            variable-integer capacity := strlen(str);

            variable-array[capacity + 1] character newCharArray;
            let newCharArray := strcpy(newCharArray, str);

            let this.arrayOfChars := newCharArray;
            //let this.capacity := capacity;
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
            //let this.capacity := totalLength;
            let this.length := totalLength;
        end concat;

        // leaks when assigning (old String object is not deleted)
        // workaround - GC can automatically free it
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

        // create replace method
        public method replace(String this)(in character ch, in character newCh) : String
            variable-integer i := 0;
            while i < this.length repeat
                if this.arrayOfChars[i] == ch then
                    this.arrayOfChars[i] := newCh;
                end if;
                let i := i + 1;
            end while;
            return this;
        end replace;

        // create insert character method with memory realloc
        public method insert(String this)(in integer index, in character ch) : String
            variable-integer i := 0;
            variable-integer j := 0;
            variable-integer newLength := this.length + 1;
            variable-array[newLength + 1] character newCharArray;
            while i < index repeat
                let newCharArray[j] := this.arrayOfChars[i];
                let i := i + 1;
                let j := j + 1;
            end while;
            let newCharArray[j] := ch;
            let j := j + 1;
            while i < this.length repeat
                let newCharArray[j] := this.arrayOfChars[i];
                let i := i + 1;
                let j := j + 1;
            end while;
            let newCharArray[j] := "\0";
            delete this.arrayOfChars;
            let this.arrayOfChars := newCharArray;
            let this.length := newLength;
            return this;
        end insert;

        public method replaceAll(String this)(in array[] character str, in array[] character newStr) : String
            variable-integer i := 0;
            variable-integer j := 0;

            variable-integer strLength := strlen(str);
            variable-integer newStrLength := strlen(newStr);
            variable-integer thisStrLength := this.length;
            variable-integer count := 0;

            // calculate number of occurrences of str in this.arrayOfChars
            while i < this.length repeat
                if this.arrayOfChars[i] == str[0] then
                    let j := 0;
                    variable-boolean failed := false;
                    while j < strLength && failed != true repeat
                        if this.arrayOfChars[i + j] != str[j] then
                            let failed := true;
                        end if;
                        if j == strLength - 1 && failed != true then
                            let count := count + 1;
                        end if;
                        let j := j + 1;
                    end while;
                end if;
                let i := i + 1;
            end while;

            variable-integer diff := newStrLength - strLength;
            variable-integer newLength := thisStrLength + diff * count;

            variable-array[newLength + 2] character newCharArray;
            let i := 0;
            let j := 0;
            variable-integer k := 0;
            // copy old string to new string and replace str with newStr
            while i < this.length repeat
                if this.arrayOfChars[i] == str[0] then
                    let k := 0;
                    variable-boolean failed := false;
                    while k < strLength && failed != true repeat
                        if this.arrayOfChars[i + k] != str[k] then
                            let failed := true;
                        end if;
                        if k == strLength - 1 && failed != true then
                            variable-integer c := 0;
                            while c < newStrLength repeat
                                let newCharArray[j + c] := newStr[c];
                                let c := c + 1;
                            end while;
                            let i := i + strLength;
                            let j := j + newStrLength;
                        end if;
                        let k := k + 1;
                    end while;
                end if;
                let newCharArray[j] := this.arrayOfChars[i];
                let i := i + 1;
                let j := j + 1;
            end while;
            let newCharArray[j] := "\0";
            delete this.arrayOfChars;
            let this.arrayOfChars := newCharArray;
            let this.length := newLength;
            return this;
        end replaceAll;

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
