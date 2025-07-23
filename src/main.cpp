// This file is part of the Comp371 Assignment 1
// It demonstrates the use of OpenGL with GLEW and GLFW to create a simple rendering
// Jeremy Crete <40246576>
// Colton Leblond <40210640>
// Charles Eimer <26747310>

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#define GLEW_STATIC 1 // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>  // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs

#include <glm/glm.hpp>                  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // GLM provides matrix transformations

static const int WIDTH = 1200;  // Window width
static const int HEIGHT = 1000; // Window height

const char *getVertexShaderSource()

{
    // For now, you use a string for your shader code, in the assignment, shaders will be stored in .glsl files
    return "#version 330 core\n"
           "layout (location = 0) in vec3 aPos;"
           "layout (location = 1) in vec3 aColor;"
           "layout (location = 2) in vec2 aTexCoord;"
           "out vec3 vertexColor;"
           "out vec2 texCoord;"
           "uniform mat4 worldMatrix;"
           "uniform mat4 viewMatrix = mat4(1.0f);"
           "uniform mat4 projectionMatrix = mat4(1.0f);"
           "void main()"
           "{"
           "   vertexColor = aColor;"
           "   gl_Position = projectionMatrix * viewMatrix * worldMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
           "   texCoord = aTexCoord;"
           "}";
}

const char *getFragmentShaderSource()
{
    return "#version 330 core\n"
           "in vec3 vertexColor;"
           "in vec2 texCoord;"
           "out vec4 FragColor;"
           "uniform sampler2D texture1;"
           "void main()"
           "{"
           "   FragColor = texture(texture1, texCoord * 50);"
           "}";
}

const char* getSkyBoxVertexShaderSource(){

    return "#version 330 core\n"
              "layout (location = 0) in vec3 aPos;"
              "out vec3 TexCoords;"
              "uniform mat4 projectionMatrix;"
              "uniform mat4 viewMatrix;"
              "void main()"
              "{"
              "   TexCoords = aPos;"
              "   vec4 pos = projectionMatrix * viewMatrix * vec4(aPos, 1.0);"
              "   gl_Position = pos.xyww;"
              "}";
}

const char* getSkyBoxFragmentShaderSource(){
    
    return "#version 330 core\n"
              "out vec4 FragColor;"
              "in vec3 TexCoords;"
              "uniform samplerCube skybox;"
              "void main()"
              "{"
              "   FragColor = texture(skybox, TexCoords);"
              "}";
}



struct TexturedColoredVertex
{
    TexturedColoredVertex(glm::vec3 _position, glm::vec3 _color, glm::vec2 _uv)
        : position(_position), color(_color), uv(_uv) {}

    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uv;
};


TexturedColoredVertex texturedSquareArray[] = {
    TexturedColoredVertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(1, 0, 0), glm::vec2(0.0f, 0.0f)),
    TexturedColoredVertex(glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f)),
    TexturedColoredVertex(glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec3(0, 0, 1), glm::vec2(1.0f, 1.0f)),

    TexturedColoredVertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(1, 0, 0), glm::vec2(0.0f, 0.0f)),
    TexturedColoredVertex(glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec3(0, 0, 1), glm::vec2(1.0f, 1.0f)),
    TexturedColoredVertex(glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec3(1, 1, 0), glm::vec2(0.0f, 1.0f))
};



float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};


std::vector<std::string> facesCubemap ={
    "Textures/skybox/px.png", //right
    "Textures/skybox/nx.png", //left
    "Textures/skybox/py.png", //top
    "Textures/skybox/ny.png", //bottom
    "Textures/skybox/pz.png", //front
    "Textures/skybox/nz.png" //back
};


int compileAndLinkShaders()
{
    // compile and link shader program
    // return shader program id
    // ------------------------------------

    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char *vertexShaderSource = getVertexShaderSource();
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragmentShaderSource = getFragmentShaderSource();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);

    glDeleteShader(fragmentShader);

    return shaderProgram;
}

//compiles the shaders fort the skybox
int compileSkyboxShaders() {

    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSource = getSkyBoxVertexShaderSource();
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentShaderSource = getSkyBoxFragmentShaderSource();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

int createTexturedVertexArrayObject(const TexturedColoredVertex* vertexArray, int arraySize)
{
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, arraySize, vertexArray, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedColoredVertex), (void*)offsetof(TexturedColoredVertex, position));
    glEnableVertexAttribArray(0);

    // Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedColoredVertex), (void*)offsetof(TexturedColoredVertex, color));
    glEnableVertexAttribArray(1);

    // Texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedColoredVertex), (void*)offsetof(TexturedColoredVertex, uv));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return vao;
}


void processInput(GLFWwindow *window, float dt, glm::vec3 &cameraPosition, const glm::vec3 &cameraLookAt, const glm::vec3 &cameraUp)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 1.5f * dt;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        cameraSpeed *= 3.0f;

    glm::vec3 right = glm::normalize(glm::cross(cameraLookAt, cameraUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += cameraSpeed * cameraLookAt;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= cameraSpeed * cameraLookAt;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= right * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += right * cameraSpeed;
}

unsigned int createSkyboxVAO(){
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    return skyboxVAO;
}

int main(int argc, char *argv[])
{

    double lastMousePosX, lastMousePosY;

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // Initialize GLFW and OpenGL version
    glfwInit();
    

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create Window and rendering context using GLFW, resolution is 800x600
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Comp371 - Assigment 1", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // removes the mouse cursor from the window

    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);


    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


    // Compile and link shaders here ...
    int shaderProgram = compileAndLinkShaders();
    int skyboxShaderProgram = compileSkyboxShaders();
    unsigned int skyboxVAO = createSkyboxVAO();

    // Define and upload geometry to the GPU here ...
    int squareAO = createTexturedVertexArrayObject(texturedSquareArray, sizeof(texturedSquareArray));


    // Variables to be used later in tutorial
    float angle = 0;
    float rotationSpeed = 180.0f; // 180 degrees per second
    float lastFrameTime = glfwGetTime();

    glm::vec3 cameraPosition(0.0f, 0.0f, 2.0f);
    glm::vec3 cameraLookAt(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

    float cameraHorizontalAngle = 90.0f;
    float cameraVerticalAngle = 0.0f;

    unsigned int cubemapTexture;
    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    stbi_set_flip_vertically_on_load(false);
    for (unsigned int i = 0; i < 6; i++){
        int width, height, nrChannels;
        unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                format,
                width,
                height,
                0,
                format,
                GL_UNSIGNED_BYTE,
                data
            );
            stbi_image_free(data);
        }
        else {
            std::cerr << "Cubemap texture failed to load at path: " << facesCubemap[i] << std::endl;
            stbi_image_free(data);
        }
    }
   

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int texWidth, texHeight, nrChannels;
    unsigned char *data = stbi_load("Textures/stone.jpg", &texWidth, &texHeight, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cerr << "Successfully loaded texture" << std::endl;
    }
    else
    {
        std::cerr << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);

    // Entering Main Loop
    while (!glfwWindowShouldClose(window))
    {

        // Frame time calculation
        float currentFrameTime = glfwGetTime();
        float dt = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        double mousePosX, mousePosY;
        glfwGetCursorPos(window, &mousePosX, &mousePosY);

        // Calculate mouse motion dx and dy
        double dx = mousePosX - lastMousePosX;
        double dy = mousePosY - lastMousePosY;

        lastMousePosX = mousePosX;
        lastMousePosY = mousePosY;

        // Update camera horizontal and vertical angle and added a constant speed for camera rotation
        const float cameraAngularSpeed = 8.0f;
        cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
        cameraVerticalAngle -= dy * cameraAngularSpeed * dt;

        // clamp vertical angle to avod flipping of the cam
        cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));

        float phi = glm::radians(cameraVerticalAngle);
        float theta = glm::radians(cameraHorizontalAngle);

        // Update camera lookAt vector based on angles
        cameraLookAt = glm::vec3(cosf(phi) * cosf(theta), sinf(phi), -cosf(phi) * sinf(theta));

        // Update camera position based on angles
        glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);

        GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);

        processInput(window, dt, cameraPosition, cameraLookAt, cameraUp);

        // Each frame, reset color of each pixel to glClearColor
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw Skybox
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(70.0f), (float)WIDTH / (float)HEIGHT, 0.01f, 100.0f);
        glDepthFunc(GL_LEQUAL);
        glUseProgram(skyboxShaderProgram);

        glm::mat4 viewMatrixSkybox = glm::mat4(glm::mat3(viewMatrix));
        GLuint viewMatrixSkyboxLocation = glGetUniformLocation(skyboxShaderProgram, "viewMatrix");
        glUniformMatrix4fv(viewMatrixSkyboxLocation, 1, GL_FALSE, &viewMatrixSkybox[0][0]);

        GLuint projectionMatrixSkyboxLocation = glGetUniformLocation(skyboxShaderProgram, "projectionMatrix");
        glUniformMatrix4fv(projectionMatrixSkyboxLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices for
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // Reset depth function

        // bind Texture
        glBindTexture(GL_TEXTURE_2D, texture);

        // Draw geometry
        glUseProgram(shaderProgram);

        

        GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

        // Draw Rectangle
        glBindVertexArray(squareAO);

        GLuint worldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");
        glm::mat4 worldMatrix = glm::mat4(1.0f);
        worldMatrix = glm::translate(worldMatrix, glm::vec3(0.0f, -1.0f, 0.0f));
        worldMatrix = glm::scale(worldMatrix, glm::vec3(500.0f, 1.0f, 500.0f));
        worldMatrix = glm::rotate(worldMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);

        glDrawArrays(GL_TRIANGLES, 0, 6); // 6 vertices, starting at index 0


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Shutdown GLFW
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}