module StdFiles
    public extern class FILE inherits Object
    end FILE;

    public extern function fopen(in array[] character filename, in array[] character mode): FILE
    end fopen;

    public extern procedure fclose(out FILE file):
    end fclose;

    public extern function fread(in array[] character buffer, in integer size, in integer count, out FILE file): integer
    end fread;

    public extern function fwrite(out FILE file, in array[] character buffer, in integer size, in integer count): integer
    end fwrite;

    public extern function fprintf(out FILE file, in array[] character format, in array[] character str): integer
    end fprintf;

    public extern function fscanf(out FILE file, in array[] character format, in array[] character str): integer
    end fscanf;

    public extern function fputc(in character c, out FILE file): character
    end fputc;

    public extern function fgetc(out FILE file): character
    end fgetc;

    public extern procedure fseek(out FILE file, in integer offset, in integer origin):
    end fseek;

    public extern function fputw(out FILE file, in integer c): integer
    end fputw;

    public extern function fgetw(out FILE file): integer
    end fgetw;

    public extern function ftell(out FILE file): integer
    end ftell;

    public extern procedure rewind(out FILE file):
    end rewind;
start
    variable-FILE file;
    let file := fopen("test.txt", "w+");
    variable-integer i := 0;
    while i < 100 repeat
        call fputc("a", file);
        let i := i + 1;
    end while;
    call fclose(file);

    let file := fopen("test.txt", "r+");
    call fseek(file, 0, 2);
    variable-integer fsize := ftell(file);
    output fsize;
    call rewind(file);

    variable-array[fsize + 1] character buffer;
    call fread(buffer, fsize, 1, file);
    call fclose(file);
    let buffer[fsize] := "\0";

    output buffer;

end StdFiles.