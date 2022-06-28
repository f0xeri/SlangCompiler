module gl
    public extern function glewInit(): integer
    end glewInit;

    public extern function glGetString(in integer name): array[] character
    end glGetString;

    public extern procedure glEnable(in integer cap):
    end glEnable;

    public extern procedure glClearColor(in float red, in float green, in float blue, in float alpha):
    end glClearColor;

    public extern procedure glClear(in integer mask):
    end glClear;

    // create glbegin
    public extern procedure glBegin(in integer mode):
    end glBegin;

    // create glend
    public extern procedure glEnd():
    end glEnd;

    public extern procedure glVertex3f(in float x, in float y, in float z):
    end glVertex3f;

    public extern procedure glColor3f(in float red, in float green, in float blue):
    end glColor3f;

    // create glLoadIdentity
    public extern procedure glLoadIdentity():
    end glLoadIdentity;

    // create glTranslatef
    public extern procedure glTranslatef(in float x, in float y, in float z):
    end glTranslatef;

    // create glDrawArrays
    public extern procedure glDrawArrays(in integer mode, in integer first, in integer count):
    end glDrawArrays;

    // create glDrawElements
    public extern procedure glDrawElements(in integer mode, in integer count, in integer type, out integer indices):
    end glDrawElements;

    // create glViewport
    public extern procedure glViewport(in integer x, in integer y, in integer width, in integer height):
    end glViewport;

    // create __glewCreateShader
    public extern variable-function(in integer): integer __glewCreateShader;

    // create __glewCompileShader
    public extern variable-procedure(in integer) __glewCompileShader;

    // create __glewShaderSource
    public extern variable-procedure(in integer, in integer, out array[] character, out integer) __glewShaderSource;

    // create __glewCreateProgram
    public extern variable-function(): integer __glewCreateProgram;

    // create __glewAttachShader
    public extern variable-function(in integer, in integer): integer __glewAttachShader;

    // create __glewLinkProgram
    public extern variable-function(in integer): integer __glewLinkProgram;

    // create __glewUseProgram
    public extern variable-function(in integer): integer __glewUseProgram;

    // create __glewGetShaderiv
    public extern variable-procedure(in integer, in integer, out integer) __glewGetShaderiv;

    // create __glewGetShaderInfoLog
    public extern variable-procedure(in integer, in integer, out integer, in array[] character) __glewGetShaderInfoLog;

    // create __glewDeleteShader
    public extern variable-procedure(in integer) __glewDeleteShader;

    // create __glewGetUniformLocation
    public extern variable-function(in integer, in array [] character): integer __glewGetUniformLocation;

    // create __glewGenVertexArrays
    public extern variable-function(in integer, out integer): integer __glewGenVertexArrays;

    // create __glewGenBuffers
    public extern variable-function(in integer, out integer): integer __glewGenBuffers;

    // create __glewBindVertexArray
    public extern variable-procedure(in integer) __glewBindVertexArray;

    // create __glewBindBuffer
    public extern variable-procedure(in integer, in integer) __glewBindBuffer;

    // create __glewBufferData
    public extern variable-procedure(in integer, in integer, in array[] float, in integer) __glewBufferData;

    // create __glewVertexAttribPointer
    public extern variable-procedure(in integer, in integer, in integer, in integer, in integer, out integer) __glewVertexAttribPointer;

    // create __glewEnableVertexAttribArray
    public extern variable-procedure(in integer) __glewEnableVertexAttribArray;

start
end gl.