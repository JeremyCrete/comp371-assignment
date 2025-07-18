// This file is part of the Comp371 Assignment 1
// It demonstrates the use of OpenGL with GLEW and GLFW to create a simple rendering
// Jeremy Crete <40246576>

#include <iostream>

#define GLEW_STATIC 1 // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>  // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs

#include <glm/glm.hpp>                  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices

#include "stb_image.h"

static const int WIDTH = 800;  // Window width
static const int HEIGHT = 600; // Window height

const char *getVertexShaderSource()
{
    // For now, you use a string for your shader code, in the assignment, shaders will be stored in .glsl files
    return "#version 330 core\n"
           "layout (location = 0) in vec3 aPos;"
           "layout (location = 1) in vec3 aColor;"
           "out vec3 vertexColor;"
           "uniform mat4 worldMatrix;"
           "uniform mat4 viewMatrix = mat4(1.0f);"
           "uniform mat4 projectionMatrix = mat4(1.0f);"
           "void main()"
           "{"
           "   vertexColor = aColor;"
           "   gl_Position = projectionMatrix * viewMatrix * worldMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
           "}";
}

const char *getFragmentShaderSource()
{
    return "#version 330 core\n"
           "in vec3 vertexColor;"
           "out vec4 FragColor;"
           "void main()"
           "{"
           "   FragColor = vec4(vertexColor.r, vertexColor.g, vertexColor.b, 1.0f);"
           "}";
}

glm::vec3 squareArray[] = {
    // First Triangle
    glm::vec3(-0.5f, -0.5f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.5f, 0.5f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(-0.5f, 0.5f, 0.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),

    // Second Triangle
    glm::vec3(0.5f, -0.5f, 0.0f),
    glm::vec3(1.0f, 1.0f, 0.0f),
    glm::vec3(0.5f, 0.5f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(-0.5f, -0.5f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
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

int createVertexArrayObject(const glm::vec3 *vertexArray, int arraySize)
{
    // Create a vertex array
    GLuint vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    // Upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
    GLuint vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, arraySize, vertexArray, GL_STATIC_DRAW);

    glVertexAttribPointer(0,                     // attribute 0 matches aPos in Vertex Shader
                          3,                     // size
                          GL_FLOAT,              // type
                          GL_FALSE,              // normalized?
                          2 * sizeof(glm::vec3), // stride - each vertex contain 2 vec3 (position, color)
                          (void *)0              // array buffer offset
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, // attribute 1 matches aColor in Vertex Shader
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          2 * sizeof(glm::vec3),
                          (void *)sizeof(glm::vec3) // color is offseted a vec3 (comes after position)
    );
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return vertexArrayObject;
}

void processInput(GLFWwindow *window)
{
    // If the user presses ESC, close the window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main(int argc, char *argv[])
{
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

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Compile and link shaders here ...
    int shaderProgram = compileAndLinkShaders();

    // Define and upload geometry to the GPU here ...
    int squareAO = createVertexArrayObject(squareArray, sizeof(squareArray));

    // Variables to be used later in tutorial
    float angle = 0;
    float rotationSpeed = 180.0f; // 180 degrees per second
    float lastFrameTime = glfwGetTime();

    glm::vec3 cameraPosition(0.0f, 0.0f, 2.0f);
    glm::vec3 cameraLookAt(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

    float cameraHorizontalAngle = 90.0f;
    float cameraVerticalAngle = 0.0f;

    double lastMousePosX, lastMousePosY;

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

        processInput(window);

        // Each frame, reset color of each pixel to glClearColor
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw geometry
        glUseProgram(shaderProgram);

        glm::mat4 projectionMatrix = glm::perspective(glm::radians(70.0f), (float)WIDTH / (float)HEIGHT, 0.01f, 100.0f);

        GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

        // Draw Rectangle
        glBindVertexArray(squareAO);

        GLuint worldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");
        glm::mat4 worldMatrix = glm::mat4(1.0f);
        glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);

        glDrawArrays(GL_TRIANGLES, 0, 6); // 6 vertices, starting at index 0

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Shutdown GLFW
    glfwTerminate();

    return 0;
}
