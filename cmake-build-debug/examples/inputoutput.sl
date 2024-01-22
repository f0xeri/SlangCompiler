import StdString;
module inputoutput
    public extern function system(in array[] character title): integer
    end system;
start
    variable-StdString.String hostname;
    call hostname.readInput(64);
    variable-StdString.String command;
    call command.init("nslookup ");
    call command.concat(hostname.toString());
    output command.toString();
    call system("chcp 65001 > nul");
    call system(command.toString());
end inputoutput;