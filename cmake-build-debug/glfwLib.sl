module GLFWLib
    public extern class GLFWmonitor inherits Object
    end GLFWmonitor;

    public extern class GLFWwindow inherits Object
    end GLFWwindow;

    public extern function glfwInit(): integer
    end glfwInit;

    public extern function glfwCreateWindow(in integer width, in integer height, in array[] character title, out GLFWmonitor monitor, out GLFWwindow share): GLFWwindow
    end glfwCreateWindow;

    public extern procedure glfwMakeContextCurrent(out GLFWwindow window)
    end glfwMakeContextCurrent;

    public extern function glfwWindowShouldClose(out GLFWwindow window): integer
    end glfwWindowShouldClose;

    public extern procedure glfwSwapBuffers(out GLFWwindow window)
    end glfwSwapBuffers;

    public extern procedure glfwPollEvents()
    end glfwPollEvents;

start
    if (glfwInit() == 0) then
        output "[ERROR] glfwInit() failed";
    end if;
    variable-GLFWwindow mainWindow;
    let mainWindow := glfwCreateWindow(800, 600, "Hello world from Slang", nil, nil);

    call glfwMakeContextCurrent(mainWindow);
    while (glfwWindowShouldClose(mainWindow) == 0) repeat
        call glfwSwapBuffers(mainWindow);
        call glfwPollEvents();
    end while;

end GLFWLib.