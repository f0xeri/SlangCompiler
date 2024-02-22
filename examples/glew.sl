module glew
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~extern~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    public extern function glewInit(): integer
    end glewInit;

    public extern function glGetString(in integer name): array[] character
    end glGetString;

    public extern procedure glEnable(in integer cap)
    end glEnable;

    public extern procedure glClearColor(in float red, in float green, in float blue, in float alpha)
    end glClearColor;

    public extern procedure glClear(in integer mask)
    end glClear;

    // create glbegin
    public extern procedure glBegin(in integer mode)
    end glBegin;

    // create glend
    public extern procedure glEnd()
    end glEnd;

    public extern procedure glVertex3f(in float x, in float y, in float z)
    end glVertex3f;

    public extern procedure glColor3f(in float red, in float green, in float blue)
    end glColor3f;

    // create glLoadIdentity
    public extern procedure glLoadIdentity()
    end glLoadIdentity;

    // create glTranslatef
    public extern procedure glTranslatef(in float x, in float y, in float z)
    end glTranslatef;

    // create glDrawArrays
    public extern procedure glDrawArrays(in integer mode, in integer first, in integer count)
    end glDrawArrays;

    // create glDrawElements
    public extern procedure glDrawElements(in integer mode, in integer count, in integer type, out character indices)
    end glDrawElements;

    // create glViewport
    public extern procedure glViewport(in integer x, in integer y, in integer width, in integer height)
    end glViewport;

    // create glPolygonMode
    public extern procedure glPolygonMode(in integer face, in integer mode)
    end glPolygonMode;

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
    public extern variable-function(in integer, out void): integer __glewGenVertexArrays;

    // create __glewGenBuffers
    public extern variable-function(in integer, out void): integer __glewGenBuffers;

    // create __glewBindVertexArray
    public extern variable-procedure(in integer) __glewBindVertexArray;

    // create __glewBindBuffer
    public extern variable-procedure(in integer, in integer) __glewBindBuffer;

    // create __glewBufferData                               // void*
    public extern variable-procedure(in integer, in integer, out void, in integer) __glewBufferData;

    // create __glewVertexAttribPointer
    public extern variable-procedure(in integer, in integer, in integer, in integer, in integer, in array[] integer) __glewVertexAttribPointer;

    // create __glewEnableVertexAttribArray
    public extern variable-procedure(in integer) __glewEnableVertexAttribArray;

    // create __glewUniform1f
    public extern variable-procedure(in integer, in float) __glewUniform1f;

    // create __glewUniform2f
    public extern variable-procedure(in integer, in float, in float) __glewUniform2f;

    // create __glewUniform3f
    public extern variable-procedure(in integer, in float, in float, in float) __glewUniform3f;

    // create __glewUniform4f
    public extern variable-procedure(in integer, in float, in float, in float, in float) __glewUniform4f;

    // create __glewUniform1i
    public extern variable-procedure(in integer, in integer) __glewUniform1i;

    // create __glewUniform2i
    public extern variable-procedure(in integer, in integer, in integer) __glewUniform2i;

    // create __glewUniform3i
    public extern variable-procedure(in integer, in integer, in integer, in integer) __glewUniform3i;

    // create __glewUniform4i
    public extern variable-procedure(in integer, in integer, in integer, in integer, in integer) __glewUniform4i;

    // create __glewUniformMatrix4fv
    public extern variable-procedure(in integer, in integer, in integer, in array[] float) __glewUniformMatrix4fv;

    // create glBindTexture
    public extern procedure glBindTexture(in integer target, in integer texture)
    end glBindTexture;

    // create glGenTextures
    public extern procedure glGenTextures(in integer n, out integer textures)
    end glGenTextures;

    // create glPixelStorei
    public extern procedure glPixelStorei(in integer pname, in integer param)
    end glPixelStorei;

    // create glTexImage2D
    public extern procedure glTexImage2D(in integer target, in integer level, in integer internalformat, in integer width, in integer height, in integer border, in integer format, in integer type, out void pixels)
    end glTexImage2D;

    // create __glewGenerateMipmap
    public extern variable-procedure(in integer) __glewGenerateMipmap;

    // create glTexParameteri
    public extern procedure glTexParameteri(in integer target, in integer pname, in integer param)
    end glTexParameteri;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~interface~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

start
end glew.