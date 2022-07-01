import glew;
import glfw;
import StdString;
import StdMath;
module openglTests
    public procedure resizeCallback(out glfw.GLFWwindow window, in integer width, in integer height):
        call glViewport(0, 0, width, height);
    end resizeCallback;

    private variable-float lastTime := 0.0f;
    private variable-integer nbFrames := 0;

    public procedure showFPS(out glfw.GLFWwindow window):
        variable-real currentTime;
        let currentTime := glfwGetTime();
        variable-real delta := currentTime - lastTime;
        let nbFrames := nbFrames + 1;
        if (delta >= 1.0) then
            variable-real fps := nbFrames / delta;
            variable-StdString.String str;
            call str.init("slang opengl 3.3 [FPS: ");
            variable-StdString.String fpsStr;
            let fpsStr := StdString.RealToString(round(fps), 0);
            call str.concat(fpsStr.toString());
            call str.concat("] ");
            call glfwSetWindowTitle(window, str.toString());
            let nbFrames := 0;
            let lastTime := currentTime;
        end if;
    end showFPS;
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
    call glfwSwapInterval(0);
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

    variable-array[12] float vertices;
    let vertices[0] :=  0.5f;  let vertices[1]  :=  0.5f;   let vertices[2]  := 0.0f;
    let vertices[3] :=  0.5f;  let vertices[4]  := -0.5f;   let vertices[5]  := 0.0f;
    let vertices[6] := -0.5f;  let vertices[7]  := -0.5f;   let vertices[8]  := 0.0f;
    let vertices[9] := -0.5f;  let vertices[10] :=  0.5f;   let vertices[11] := 0.0f;

    variable-array[6] integer indices;
    let indices[0] := 0; let indices[1] := 1; let indices[2] := 3;
    let indices[3] := 1; let indices[4] := 2; let indices[5] := 3;

    variable-integer vertexBufferObject := 0;
    variable-integer vertexArrayObject := 0;
    variable-integer indexBufferObject := 0;

    call __glewGenVertexArrays(1, vertexArrayObject);
    call __glewGenBuffers(1, vertexBufferObject);
    call __glewGenBuffers(1, indexBufferObject);

    call __glewBindVertexArray(vertexArrayObject);

    call __glewBindBuffer(34962, vertexBufferObject);
    call __glewBufferData(34962, 48, vertices, 35044);

    call __glewBindBuffer(34963, indexBufferObject);
    call __glewBufferData(34963, 24, indices, 35044);

    call __glewVertexAttribPointer(0, 3, 5126, 0, 3 * 4, nil);
    call __glewEnableVertexAttribArray(0);

    call __glewBindBuffer(34962, 0);
    call __glewBindVertexArray(0);

    while (glfwWindowShouldClose(mainWindow) == 0) repeat
        call showFPS(mainWindow);

        call glClear(16384);
        call glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        call __glewUseProgram(shaderProgram);
        call __glewBindVertexArray(vertexArrayObject);
        // call glDrawArrays(4, 0, 3);
        call glDrawElements(4, 6, 5125, nil);

        call glfwSwapBuffers(mainWindow);
        call glfwPollEvents();
    end while;
end openglTests.