import StdString;
import StdMath;
import glew;
import glfw;
import GLTexture;
import matrix4x4f;

module openglTests
    public variable-integer _width := 800;
    public variable-integer _height := 600;
    public procedure resizeCallback(out GLFWwindow window, in integer width, in integer height)
        call glViewport(0, 0, width, height);
        let _width := width;
        let _height := height;
    end resizeCallback;

    private variable-float lastTime := 0.0f;
    private variable-integer nbFrames := 0;
    public variable-StdString.String str;
    public procedure showFPS(out GLFWwindow window)
        variable-real currentTime := glfwGetTime();
        variable-real delta := currentTime - lastTime;
        let nbFrames := nbFrames + 1;
        if (delta >= 1.0) then
            variable-real fps := nbFrames / delta;
            //variable-StdString.String str;
            call str.init("slang opengl 3.3 [FPS: ");
            call str.concat(StdString.RealToCharArray(round(fps), 0));
            call str.concat("] ");
            call glfwSetWindowTitle(window, str.toString());
            let nbFrames := 0;
            let lastTime := currentTime;
            call str.clear();
        end if;
    end showFPS;
start

    if (glfwInit() == 0) then
        output "[ERROR] glfwInit() failed";
    end if;
    variable-GLFWwindow mainWindow := glfwCreateWindow(_width, _height, "slang triangle opengl 3.3", nil, nil);
    variable-procedure(out GLFWwindow, in integer, in integer) resizeCallbackPtr := resizeCallback;
    call glfwSetFramebufferSizeCallback(mainWindow, resizeCallbackPtr);
    call glfwMakeContextCurrent(mainWindow);
    call glfwSwapInterval(0);
    call glEnable(3042); // gl blend
    call glEnable(2848); // gl cull face
    call glEnable(2929); // gl depth test
    if (glewInit() != 0) then
        output "[ERROR] glewInit() failed";
    end if;
    //variable-array[0] character vertexShaderSource := "#version 330 core\nlayout (location = 0) in vec3 aPos;\nlayout (location = 1) in vec2 aTexCoord;\nlayout (location = 2) in vec3 aNormal;\nout vec2 TexCoord;\nout vec3 Normal;\nuniform mat4 model;\nuniform mat4 view;\nuniform mat4 projection;\nvoid main()\n{\ngl_Position = projection * view * model * vec4(aPos, 1.0);\nTexCoord = aTexCoord;\nNormal = mat3(transpose(inverse(model))) * aNormal;\n}\0";
    //variable-array[0] character fragmentShaderSource := "#version 330 core\nout vec4 FragColor;\nin vec2 TexCoord;\nin vec3 Normal;\nuniform sampler2D texture1;\nvoid main()\n{\nvec3 lightDir = normalize(vec3(0.5, 0.6, 0.4));\nfloat ambientStrength = 0.2;\nvec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);\nfloat diffuseStrength = max(dot(normalize(Normal), lightDir), 0.0);\nvec3 diffuse = diffuseStrength * vec3(1.0, 1.0, 1.0);\nvec3 result = (ambient + diffuse) * texture2D(texture1, TexCoord).rgb;\nFragColor = vec4(result, 1.0);\n}\0";

    variable-array[0] character vertexShaderSource := "#version 330 core\nlayout (location = 0) in vec3 aPos;\nlayout (location = 1) in vec2 aTexCoord;\nlayout (location = 2) in vec3 aNormal;\nout vec2 TexCoord;\nout vec3 FragPos;\nout vec3 Normal;\nuniform mat4 model;\nuniform mat4 view;\nuniform mat4 projection;\nvoid main()\n{\ngl_Position = projection * view * model * vec4(aPos, 1.0);\nTexCoord = aTexCoord;\nFragPos = vec3(model * vec4(aPos, 1.0));\nNormal = mat3(transpose(inverse(model))) * aNormal;\n}\0";
    variable-array[0] character fragmentShaderSource := "#version 330 core\nout vec4 FragColor;\nin vec2 TexCoord;\nin vec3 FragPos;\nin vec3 Normal;\nuniform sampler2D texture1;\nvec3 lightPos = vec3(-3, 1, 3);\nvec3 viewPos = vec3(0.0, 0.0, 3.0);\nfloat shininess = 32.0;\nvoid main()\n{\nvec3 ambient = 0.45f * vec3(1.0f, 1.0f, 1.0f);\nvec3 norm = normalize(Normal);\nvec3 lightDir = normalize(lightPos - FragPos);\nfloat diff = max(dot(norm, lightDir), 0.0);\nvec3 diffuse = vec3(1.0, 1.0, 1.0) * diff;\nvec3 viewDir = normalize(viewPos - FragPos);\nvec3 halfwayDir = normalize(lightDir + viewDir);\nfloat spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);\nvec3 specular = vec3(1.0, 1.0, 1.0) * spec;\nvec3 result = ambient + diffuse + specular;\nFragColor = vec4(result * texture2D(texture1, TexCoord).rgb, 1.0);\n}\0";

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

    variable-GLTexture.Texture texture;
    call texture.load("cat.png");

    variable-array[72] float vertices;
    let vertices[0] :=  0.0f;  let vertices[1]  :=  1.0f;   let vertices[2]  := 1.0f;
    let vertices[3] :=  1.0f;  let vertices[4]  :=  1.0f;   let vertices[5]  := 1.0f;
    let vertices[6] :=  1.0f;  let vertices[7]  :=  1.0f;   let vertices[8]  := 0.0f;
    let vertices[9] :=  0.0f;  let vertices[10] :=  1.0f;   let vertices[11] := 0.0f;

    let vertices[12] := 1.0f;  let vertices[13] :=  0.0f;   let vertices[14] := 0.0f;
    let vertices[15] := 0.0f;  let vertices[16] :=  0.0f;   let vertices[17] := 0.0f;
    let vertices[18] := 0.0f;  let vertices[19] :=  1.0f;   let vertices[20] := 0.0f;
    let vertices[21] := 1.0f;  let vertices[22] :=  1.0f;   let vertices[23] := 0.0f;

    let vertices[24] := 0.0f;  let vertices[25] :=  0.0f;   let vertices[26] := 1.0f;
    let vertices[27] := 1.0f;  let vertices[28] :=  0.0f;   let vertices[29] := 1.0f;
    let vertices[30] := 1.0f;  let vertices[31] :=  1.0f;   let vertices[32] := 1.0f;
    let vertices[33] := 0.0f;  let vertices[34] :=  1.0f;   let vertices[35] := 1.0f;

    let vertices[36] := 0.0f;  let vertices[37] :=  0.0f;   let vertices[38] := 0.0f;
    let vertices[39] := 1.0f;  let vertices[40] :=  0.0f;   let vertices[41] := 0.0f;
    let vertices[42] := 1.0f;  let vertices[43] :=  0.0f;   let vertices[44] := 1.0f;
    let vertices[45] := 0.0f;  let vertices[46] :=  0.0f;   let vertices[47] := 1.0f;

    let vertices[48] := 1.0f;  let vertices[49] :=  0.0f;   let vertices[50] := 1.0f;
    let vertices[51] := 1.0f;  let vertices[52] :=  0.0f;   let vertices[53] := 0.0f;
    let vertices[54] := 1.0f;  let vertices[55] :=  1.0f;   let vertices[56] := 0.0f;
    let vertices[57] := 1.0f;  let vertices[58] :=  1.0f;   let vertices[59] := 1.0f;

    let vertices[60] := 0.0f;  let vertices[61] :=  0.0f;   let vertices[62] := 0.0f;
    let vertices[63] := 0.0f;  let vertices[64] :=  0.0f;   let vertices[65] := 1.0f;
    let vertices[66] := 0.0f;  let vertices[67] :=  1.0f;   let vertices[68] := 1.0f;
    let vertices[69] := 0.0f;  let vertices[70] :=  1.0f;   let vertices[71] := 0.0f;

    // normals
    variable-float dImin := 0.0f;
    variable-float dImax := 1.0f;
    variable-array[72] float normals;
    let normals[0] := dImin;   let normals[1] := dImax;   let normals[2] := dImin;
    let normals[3] := dImin;   let normals[4] := dImax;   let normals[5] := dImin;
    let normals[6] := dImin;   let normals[7] := dImax;   let normals[8] := dImin;
    let normals[9] := dImin;   let normals[10] := dImax;  let normals[11] := dImin;

    let normals[12] := dImin;  let normals[13] := dImin;  let normals[14] := -dImax;
    let normals[15] := dImin;  let normals[16] := dImin;  let normals[17] := -dImax;
    let normals[18] := dImin;  let normals[19] := dImin;  let normals[20] := -dImax;
    let normals[21] := dImin;  let normals[22] := dImin;  let normals[23] := -dImax;

    let normals[24] := dImin;  let normals[25] := dImin;  let normals[26] := dImax;
    let normals[27] := dImin;  let normals[28] := dImin;  let normals[29] := dImax;
    let normals[30] := dImin;  let normals[31] := dImin;  let normals[32] := dImax;
    let normals[33] := dImin;  let normals[34] := dImin;  let normals[35] := dImax;

    let normals[36] := dImin;  let normals[37] := -dImax; let normals[38] := dImin;
    let normals[39] := dImin;  let normals[40] := -dImax; let normals[41] := dImin;
    let normals[42] := dImin;  let normals[43] := -dImax; let normals[44] := dImin;
    let normals[45] := dImin;  let normals[46] := -dImax; let normals[47] := dImin;

    let normals[48] := dImax;  let normals[49] := dImin;  let normals[50] := dImin;
    let normals[51] := dImax;  let normals[52] := dImin;  let normals[53] := dImin;
    let normals[54] := dImax;  let normals[55] := dImin;  let normals[56] := dImin;
    let normals[57] := dImax;  let normals[58] := dImin;  let normals[59] := dImin;

    let normals[60] := -dImax; let normals[61] := dImin;  let normals[62] := dImin;
    let normals[63] := -dImax; let normals[64] := dImin;  let normals[65] := dImin;
    let normals[66] := -dImax; let normals[67] := dImin;  let normals[68] := dImin;
    let normals[69] := -dImax; let normals[70] := dImin;  let normals[71] := dImin;

    variable-array[48] float texCoords;
    let texCoords[0] := 0.0f; let texCoords[1] := 1.0f;
    let texCoords[2] := 1.0f; let texCoords[3] := 1.0f;
    let texCoords[4] := 1.0f; let texCoords[5] := 0.0f;
    let texCoords[6] := 0.0f; let texCoords[7] := 0.0f;

    let texCoords[8] := 0.0f; let texCoords[9] := 1.0f;
    let texCoords[10] := 1.0f; let texCoords[11] := 1.0f;
    let texCoords[12] := 1.0f; let texCoords[13] := 0.0f;
    let texCoords[14] := 0.0f; let texCoords[15] := 0.0f;

    let texCoords[16] := 0.0f; let texCoords[17] := 1.0f;
    let texCoords[18] := 1.0f; let texCoords[19] := 1.0f;
    let texCoords[20] := 1.0f; let texCoords[21] := 0.0f;
    let texCoords[22] := 0.0f; let texCoords[23] := 0.0f;

    let texCoords[24] := 0.0f; let texCoords[25] := 1.0f;
    let texCoords[26] := 1.0f; let texCoords[27] := 1.0f;
    let texCoords[28] := 1.0f; let texCoords[29] := 0.0f;
    let texCoords[30] := 0.0f; let texCoords[31] := 0.0f;

    let texCoords[32] := 0.0f; let texCoords[33] := 1.0f;
    let texCoords[34] := 1.0f; let texCoords[35] := 1.0f;
    let texCoords[36] := 1.0f; let texCoords[37] := 0.0f;
    let texCoords[38] := 0.0f; let texCoords[39] := 0.0f;

    let texCoords[40] := 0.0f; let texCoords[41] := 1.0f;
    let texCoords[42] := 1.0f; let texCoords[43] := 1.0f;
    let texCoords[44] := 1.0f; let texCoords[45] := 0.0f;
    let texCoords[46] := 0.0f; let texCoords[47] := 0.0f;

    variable-array[36] integer indices;
    let indices[0] := 0; let indices[1] := 1; let indices[2] := 2;
    let indices[3] := 2; let indices[4] := 3; let indices[5] := 0;

    let indices[6] := 4; let indices[7] := 5; let indices[8] := 6;
    let indices[9] := 6; let indices[10] := 7; let indices[11] := 4;

    let indices[12] := 8; let indices[13] := 9; let indices[14] := 10;
    let indices[15] := 10; let indices[16] := 11; let indices[17] := 8;

    let indices[18] := 12; let indices[19] := 13; let indices[20] := 14;
    let indices[21] := 14; let indices[22] := 15; let indices[23] := 12;

    let indices[24] := 16; let indices[25] := 17; let indices[26] := 18;
    let indices[27] := 18; let indices[28] := 19; let indices[29] := 16;

    let indices[30] := 20; let indices[31] := 21; let indices[32] := 22;
    let indices[33] := 22; let indices[34] := 23; let indices[35] := 20;

    variable-integer vertexBufferObject := 0;
    variable-integer vertexArrayObject := 0;
    variable-integer indexBufferObject := 0;
    variable-integer texCoordsObject := 0;
    variable-integer normalBufferObject := 0;

    call __glewGenVertexArrays(1, vertexArrayObject);
    call __glewGenBuffers(1, vertexBufferObject);
    call __glewGenBuffers(1, texCoordsObject);
    call __glewGenBuffers(1, normalBufferObject);
    call __glewGenBuffers(1, indexBufferObject);

    call __glewBindVertexArray(vertexArrayObject);

    call __glewBindBuffer(34962, vertexBufferObject);
    call __glewBufferData(34962, 288, vertices, 35044);
    call __glewVertexAttribPointer(0, 3, 5126, 0, 3 * 4, nil);
    call __glewEnableVertexAttribArray(0);

    call __glewBindBuffer(34962, texCoordsObject);
    call __glewBufferData(34962, 192, texCoords, 35044);
    call __glewVertexAttribPointer(1, 2, 5126, 0, 2 * 4, nil);
    call __glewEnableVertexAttribArray(1);

    call __glewBindBuffer(34962, normalBufferObject);
    call __glewBufferData(34962, 288, normals, 35044);
    call __glewVertexAttribPointer(2, 3, 5126, 0, 3 * 4, nil);
    call __glewEnableVertexAttribArray(2);

    call __glewBindBuffer(34963, indexBufferObject);
    call __glewBufferData(34963, 144, indices, 35044);

    call __glewBindBuffer(34962, 0);
    call __glewBindVertexArray(0);

    variable-matrix4x4f.Matrix4 matrix;
    call matrix.init(1.0f);

    variable-matrix4x4f.Matrix4 view;
    variable-matrix4x4f.Matrix4 projection;

    variable-integer transformLoc := __glewGetUniformLocation(shaderProgram, "model");
    variable-integer projLoc := __glewGetUniformLocation(shaderProgram, "projection");
    variable-integer viewLoc := __glewGetUniformLocation(shaderProgram, "view");
    variable-float time := glfwGetTime();

    while (glfwWindowShouldClose(mainWindow) == 0) repeat
        call showFPS(mainWindow);

        call glClear(16640);

        call glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        let time := glfwGetTime();

        call matrix.init(1.0f);
        call matrix.translate(-0.5f, -0.5f, -0.5f);
        call matrix.rotate(time, 0, 1, 0);
        call matrix.rotate(time, 0, 0, 1);
	    call matrix.rotate(time, 1, 0, 0);

        call texture.bind();
        call __glewUseProgram(shaderProgram);

        call view.init(1.0f);
        call view.translate(0.0f, 0.0f, -3.0f);
        call projection.perspective(45.0f, _width * 1.0 / _height, 0.1f, 100.0f);
        call __glewUniformMatrix4fv(projLoc, 1, 0, projection.rawData());
        call __glewUniformMatrix4fv(viewLoc, 1, 0, view.rawData());
        call __glewUniformMatrix4fv(transformLoc, 1, 0, matrix.rawData());
        call __glewBindVertexArray(vertexArrayObject);
        // call glDrawArrays(4, 0, 3);

        call glDrawElements(4, 36, 5125, nil);

        call glfwSwapBuffers(mainWindow);
        call glfwPollEvents();
    end while;
end openglTests.