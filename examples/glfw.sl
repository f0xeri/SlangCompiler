module glfw
    public extern class GLFWmonitor inherits Object
    end GLFWmonitor;

    public extern class GLFWwindow inherits Object
    end GLFWwindow;

    public extern function glfwInit(): integer
    end glfwInit;

    public extern function glfwCreateWindow(in integer width, in integer height, in array[] character title, out GLFWmonitor monitor, out GLFWwindow share): GLFWwindow
    end glfwCreateWindow;

    public extern function glfwSwapInterval(in integer interval): integer
    end glfwSwapInterval;

    public extern procedure glfwMakeContextCurrent(out GLFWwindow window)
    end glfwMakeContextCurrent;

    public extern function glfwWindowShouldClose(out GLFWwindow window): integer
    end glfwWindowShouldClose;

    public extern procedure glfwSwapBuffers(out GLFWwindow window)
    end glfwSwapBuffers;

    public extern procedure glfwPollEvents()
    end glfwPollEvents;

    public extern procedure glfwGetWindowSize(out GLFWwindow window, out integer width, out integer height)
    end glfwGetWindowSize;

    public extern function glfwGetTime(): real
    end glfwGetTime;

    public extern procedure glfwSetWindowTitle(out GLFWwindow window, in array[] character title)
    end glfwSetWindowTitle;

    // create glfwSetFramebufferSizeCallback
    public extern procedure glfwSetFramebufferSizeCallback(out GLFWwindow window, in procedure(out GLFWwindow, in integer, in integer) callback)
    end glfwSetFramebufferSizeCallback;
start

end glfw.