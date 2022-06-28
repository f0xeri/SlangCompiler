import glew;
import glfw;
module openglTests
    public procedure resizeCallback(out glfw.GLFWwindow window, in integer width, in integer height):
        call glViewport(0, 0, width, height);
    end resizeCallback;

    public procedure resizeCallback():

    end resizeCallback;
start
    if (glfwInit() == 0) then
        output "[ERROR] glfwInit() failed";
    end if;
    variable-glfw.GLFWwindow mainWindow;
    let mainWindow := glfwCreateWindow(800, 600, "slang triangle opengl 3.3", nil, nil);
    variable-procedure(out glfw.GLFWwindow, in integer, in integer) resizeCallbackPtr;
    let resizeCallbackPtr := resizeCallback;
    call glfwSetFramebufferSizeCallback(mainWindow, resizeCallbackPtr);
    call glfwMakeContextCurrent(mainWindow);
    call glEnable(3042); // gl blend
    if (glewInit() != 0) then
        output "[ERROR] glewInit() failed";
    end if;
    variable-array[128] character vertexShaderSource;
    let vertexShaderSource := "#version 330 core\nlayout (location = 0) in vec3 aPos;\nvoid main()\n{\ngl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n}\0";
    variable-array[128] character fragmentShaderSource;
    let fragmentShaderSource := "#version 330 core\nout vec4 FragColor;\nvoid main()\n{\nFragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n}\0";

    variable-integer vertexShader := __glewCreateShader(35633); // GL_VERTEX_SHADER;
    call __glewShaderSource(vertexShader, 1, vertexShaderSource, nil);
    call __glewCompileShader(vertexShader);

    variable-integer fragmentShader := __glewCreateShader(35632); // GL_VERTEX_SHADER;
    call __glewShaderSource(fragmentShader, 1, fragmentShaderSource, nil);
    call __glewCompileShader(fragmentShader);

    variable-integer shaderProgram := __glewCreateProgram();
    call __glewAttachShader(shaderProgram, vertexShader);
    call __glewAttachShader(shaderProgram, fragmentShader);
    call __glewLinkProgram(shaderProgram);

    call __glewDeleteShader(vertexShader);
    call __glewDeleteShader(fragmentShader);

    variable-array[9] float vertices;
    let vertices[0] := -0.5f;
    let vertices[1] := -0.5f;
    let vertices[2] := 0.0f;
    let vertices[3] := 0.5f;
    let vertices[4] := -0.5f;
    let vertices[5] := 0.0f;
    let vertices[6] := 0.0f;
    let vertices[7] := 0.5f;
    let vertices[8] := 0.0f;

    variable-integer vertexBufferObject := 0;
    variable-integer vertexArrayObject := 0;

    call __glewGenVertexArrays(1, vertexArrayObject);
    call __glewGenBuffers(1, vertexBufferObject);

    call __glewBindVertexArray(vertexArrayObject);

    call __glewBindBuffer(34962, vertexBufferObject);
    call __glewBufferData(34962, 36, vertices, 35044);

    call __glewVertexAttribPointer(0, 3, 5126, 0, 3 * 4, nil);
    call __glewEnableVertexAttribArray(0);

    call __glewBindBuffer(34962, 0);
    call __glewBindVertexArray(0);

    while (glfwWindowShouldClose(mainWindow) == 0) repeat
        call glClear(16384);
        call glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        call __glewUseProgram(shaderProgram);
        call __glewBindVertexArray(vertexArrayObject);
        call glDrawArrays(4, 0, 3);

        call glfwSwapBuffers(mainWindow);
        call glfwPollEvents();
    end while;
end openglTests.