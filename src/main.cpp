// This file is part of the Comp371 Assignment 2
// It demonstrates the use of OpenGL with GLEW and GLFW to create a simple rendering and the use of assimp to load obj files 
// Jeremy Crete <40246576>
// Colton Leblond <40210640>
// Charles Eimer <26747310>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>




#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define GLEW_STATIC 1 // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>  // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs

// GLM is an optimized math library with syntax to similar to OpenGL Shading Language
// GLM provides matrix transformations
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#else
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

#include "ModelLoader.h"
#include "ParticleSystem.h"


glm::vec3 currentModelColor(0.0f, 0.0f, 0.0f); // Start with black
int currentTextureMode = 1;

bool autoSpinMode = false;
float spinAngle = 0.0f;
float spinSpeed = 30.0f;

float carSpinAngle = 0.0f;
float carSpinSpeed = 20.0f; //car

float wheelSpinAngle = 0.0f;
float wheelSpinSpeed = 240.0f; //wheels

glm::vec3 dirLightPos(0.0f, 30.0f, -40.0f);

float dirLightAngle = 0.0f;
float dirLightSpeed = 20.0f; // degrees per second
float dirLightRadius = 40.0f; // match initial Z value
float dirLightHeight = 30.0f; // match initial Y value

glm::vec3 dirLightPos(0.0f, 30.0f, -40.0f);

float dirLightAngle = 0.0f;
float dirLightSpeed = 20.0f; // degrees per second
float dirLightRadius = 40.0f; // match initial Z value
float dirLightHeight = 30.0f; // match initial Y value


// f = autospin around model
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)  {
        autoSpinMode = !autoSpinMode;
        if (autoSpinMode) spinAngle = 0.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        currentTextureMode = 1; // Onyx
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        currentTextureMode = 2; // Gold
}


// positions of the point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(1.0f, 10.0f, 1.0f),

};

// used to import model using assimp

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

void Mesh::Draw() const
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void processMesh(aiMesh* mesh, std::vector<Mesh>& meshes)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Mesh newMesh;
    newMesh.name = mesh->mName.C_Str();

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex;
        vertex.Position = glm::vec3(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z);
        vertex.Normal = mesh->HasNormals()
            ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
            : glm::vec3(0.0f);
        vertex.TexCoords = mesh->mTextureCoords[0]
            ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
            : glm::vec2(0.0f);
        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    newMesh.vertices = std::move(vertices);
    newMesh.indices = std::move(indices);
    newMesh.setupMesh();
    meshes.push_back(std::move(newMesh));
}

void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes)
{
    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, meshes);
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        processNode(node->mChildren[i], scene, meshes);
    }
}

std::vector<Mesh> loadModel(const std::string& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << std::endl;
        return {};
    }

    std::vector<Mesh> meshes;
    processNode(scene->mRootNode, scene, meshes);
    return meshes;
}

static const int WIDTH = 1200;  // Window width
static const int HEIGHT = 1000; // Window height

std::string loadShaderSource(const std::string& filepath)
{
    std::ifstream file(filepath);
    std::stringstream buffer;
    if (file.is_open())
    {
        buffer << file.rdbuf();
        file.close();
    }
    else
    {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
    }
    return buffer.str();
}

#if defined(WIN32) || defined(_WIN32)
std::string vertexShaderStr = loadShaderSource("src\\Shaders\\shader.vert");
const char* vertexShaderSource = vertexShaderStr.c_str();

std::string fragmentShaderStr = loadShaderSource("src\\Shaders\\shader.frag");
const char* fragmentShaderSource = fragmentShaderStr.c_str();

std::string skyboxVertexShaderStr = loadShaderSource("src\\Shaders\\skybox.vert");
const char* skyboxVertexShaderSource = skyboxVertexShaderStr.c_str();

std::string skyboxFragmentShaderStr = loadShaderSource("src\\Shaders\\skybox.frag");
const char* skyboxFragmentShaderSource = skyboxFragmentShaderStr.c_str();

std::string vertexShader2Str = loadShaderSource("src\\Shaders\\shader2.vert");
const char* vertexShader2Source = vertexShader2Str.c_str();

std::string fragment2ShaderStr = loadShaderSource("src\\Shaders\\shader2.frag");
const char* fragment2ShaderSource = fragment2ShaderStr.c_str();

std::string vertexParticleShaderStr = loadShaderSource("src\\Shaders\\particle.vert");
const char* vertexParticleShaderSource = vertexParticleShaderStr.c_str();

std::string fragmentParticleShaderStr = loadShaderSource("src\\Shaders\\particle.frag");
const char* fragmentParticleShaderSource = fragmentParticleShaderStr.c_str();

#else
std::string vertexShaderStr = loadShaderSource("Shaders/shader.vert");
const char* vertexShaderSource = vertexShaderStr.c_str();

std::string fragmentShaderStr = loadShaderSource("Shaders/shader.frag");
const char* fragmentShaderSource = fragmentShaderStr.c_str();

std::string skyboxVertexShaderStr = loadShaderSource("Shaders/skybox.vert");
const char* skyboxVertexShaderSource = skyboxVertexShaderStr.c_str();

std::string skyboxFragmentShaderStr = loadShaderSource("Shaders/skybox.frag");
const char* skyboxFragmentShaderSource = skyboxFragmentShaderStr.c_str();

std::string vertexShader2Str = loadShaderSource("Shaders/shader2.vert");
const char* vertexShader2Source = vertexShader2Str.c_str();

std::string fragment2ShaderStr = loadShaderSource("Shaders/shader2.frag");
const char* fragment2ShaderSource = fragment2ShaderStr.c_str();
#endif

struct TexturedColoredVertex
{
    TexturedColoredVertex(glm::vec3 _position, glm::vec3 _color, glm::vec2 _uv, glm::vec3 _normal)
        : position(_position), color(_color), uv(_uv), normal(_normal)
    {
    }

    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uv;
    glm::vec3 normal;
};

TexturedColoredVertex texturedSquareArray[] = {
    // First triangle
    TexturedColoredVertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(1, 0, 0), glm::vec2(0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)),
    TexturedColoredVertex(glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)),
    TexturedColoredVertex(glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0, 0, 1), glm::vec2(1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f)),

    // Second triangle
    TexturedColoredVertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(1, 0, 0), glm::vec2(0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)),
    TexturedColoredVertex(glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0, 0, 1), glm::vec2(1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f)),
    TexturedColoredVertex(glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(1, 1, 0), glm::vec2(0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f)),

};

float skyboxVertices[] = {
    // positions
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f };

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
std::vector<std::string> facesCubemap = {
    "src\\Textures\\skybox\\px.png", // right
    "src\\Textures\\skybox\\nx.png", // left
    "src\\Textures\\skybox\\py.png", // top
    "src\\Textures\\skybox\\ny.png", // bottom
    "src\\Textures\\skybox\\pz.png", // front
    "src\\Textures\\skybox\\nz.png"  // back
};
#else
std::vector<std::string> facesCubemap = {
    "Textures/skybox/px.png", // right
    "Textures/skybox/nx.png", // left
    "Textures/skybox/py.png", // top
    "Textures/skybox/ny.png", // bottom
    "Textures/skybox/pz.png", // front
    "Textures/skybox/nz.png"  // back
};
#endif

int compileAndLinkShaders(const char* vertexSource, const char* fragmentSource)
{
    // compile and link shader program
    // return shader program id
    // ------------------------------------

    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexSource, NULL);
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

    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
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

// compiles the shaders fort the skybox
int compileSkyboxShaders()
{

    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &skyboxVertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &skyboxFragmentShaderSource, NULL);
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

    // Normals
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedColoredVertex), (void*)offsetof(TexturedColoredVertex, normal));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return vao;
}

unsigned int createSkyboxVAO()
{
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

unsigned int createLightVAO()
{
    unsigned int lightVAO, lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    return lightVAO;
}

void processInput(GLFWwindow* window, float dt, glm::vec3& cameraPosition, const glm::vec3& cameraLookAt, const glm::vec3& cameraUp)
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

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        currentTextureMode = 1; // Onyx
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        currentTextureMode = 2; // Gold
}

// center of model to help with spin
static glm::vec3 meshCenter(const Mesh& m) {
    if (m.vertices.empty()) return glm::vec3(0.0f);
    glm::vec3 mn = m.vertices[0].Position, mx = m.vertices[0].Position;
    for (size_t i = 1; i < m.vertices.size(); ++i) {
        const glm::vec3 p = m.vertices[i].Position;
        if (p.x < mn.x) mn.x = p.x; if (p.y < mn.y) mn.y = p.y; if (p.z < mn.z) mn.z = p.z;
        if (p.x > mx.x) mx.x = p.x; if (p.y > mx.y) mx.y = p.y; if (p.z > mx.z) mx.z = p.z;
    }
    return (mn + mx) * 0.5f;
}


int main(int argc, char* argv[])
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

#ifdef __APPLE__
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif

    // Create Window and rendering context using GLFW, resolution is 800x600
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Comp371 - Assigment 2", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, keyCallback);

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

    //shadow map
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    GLuint depthMapFBO = 0, depthMapTex = 0;

    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMapTex);
    glBindTexture(GL_TEXTURE_2D, depthMapTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    {
        float borderCol[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderCol);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    std::string modelVertSrc = loadShaderSource("src\\Shaders\\model.vert");
	std::string modelFragSrc = loadShaderSource("src\\Shaders\\model.frag");    
#else
    std::string modelVertSrc = loadShaderSource("Shaders/model.vert");
    std::string modelFragSrc = loadShaderSource("Shaders/model.frag");
#endif
    const char* modelVert = modelVertSrc.c_str();
    const char* modelFrag = modelFragSrc.c_str();
    int modelShaderProgram = compileAndLinkShaders(modelVert, modelFrag);

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    int depthShader = compileAndLinkShaders(
        loadShaderSource("src\\Shaders\\shadow_depth.vert").c_str(),
        loadShaderSource("src\\Shaders\\shadow_depth.frag").c_str()
    );
    int floorShadowProgram = compileAndLinkShaders(
        loadShaderSource("src\\Shaders\\floor_shadow.vert").c_str(),
        loadShaderSource("src\\Shaders\\floor_shadow.frag").c_str()
	);
#else
    int depthShader = compileAndLinkShaders(
        loadShaderSource("Shaders/shadow_depth.vert").c_str(),
        loadShaderSource("Shaders/shadow_depth.frag").c_str()
    );

    int floorShadowProgram = compileAndLinkShaders(
        loadShaderSource("Shaders/floor_shadow.vert").c_str(),
        loadShaderSource("Shaders/floor_shadow.frag").c_str()
    );
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	std::vector<Mesh> cubeMeshes = loadModel("src\\Models\\cyberCar.obj");
#else
    std::vector<Mesh> cubeMeshes = loadModel("Models/cyberCar.obj");
#endif

    // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Compile and link shaders here ...
    int shaderProgram = compileAndLinkShaders(vertexShaderSource, fragmentShaderSource);
    int groundVAO = createTexturedVertexArrayObject(texturedSquareArray, sizeof(texturedSquareArray));

    int skyboxShaderProgram = compileSkyboxShaders();
    unsigned int skyboxVAO = createSkyboxVAO();


	//Init particle systems
	std::vector<ParticleSystem*> particleSystems;

    for (int i = 0; i < 4; i++)
    {
        ParticleSystem* particleSystem = new ParticleSystem();
        particleSystem->compileAndLinkShaders(vertexParticleShaderSource, fragmentParticleShaderSource);
        particleSystem->createQuad();

		particleSystems.push_back(particleSystem);
    }

    // Compile and link shaders for the light cube
    int lightCubeProgram = compileAndLinkShaders(vertexShader2Source, fragment2ShaderSource);
    int cubeVAO = createLightVAO();

    // Variables to be used later in tutorial
    float angle = 0;
    float rotationSpeed = 180.0f; // 180 degrees per second
    float lastFrameTime = glfwGetTime();

    glm::vec3 cameraPosition(0.0f, 8.0f, 15.0f);
    glm::vec3 cameraLookAt(0.0f, 0.0f, 0.0f);
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
    for (unsigned int i = 0; i < 6; i++)
    {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
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
                data);
            stbi_image_free(data);
        }
        else
        {
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

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    unsigned char* data = stbi_load("src\\Textures\\stone.jpg", &texWidth, &texHeight, &nrChannels, 0);
#else
    unsigned char* data = stbi_load("Textures/stone.jpg", &texWidth, &texHeight, &nrChannels, 0);
#endif //

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

    // load onyx Bake from blender
    unsigned int cyberTexture;
    glGenTextures(1, &cyberTexture);
    glBindTexture(GL_TEXTURE_2D, cyberTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int cyberWidth, cyberHeight, cyberChannels;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	unsigned char* cyberData = stbi_load("src\\Models\\cyberBake.png", &cyberWidth, &cyberHeight, &cyberChannels, 0);
#else
    unsigned char* cyberData = stbi_load("models/cyberBake.png", &cyberWidth, &cyberHeight, &cyberChannels, 0);
#endif
    if (cyberData)
    {
        GLenum format = (cyberChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, cyberWidth, cyberHeight, 0, format, GL_UNSIGNED_BYTE, cyberData);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cerr << "Successfully loaded cyberBake.png" << std::endl;
    }
    else
    {
        std::cerr << "Failed to load cyberBake.png" << std::endl;
    }
    stbi_image_free(cyberData);

    // Load Gold Color Map
    unsigned int goldTexture;
    glGenTextures(1, &goldTexture);
    glBindTexture(GL_TEXTURE_2D, goldTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int goldWidth, goldHeight, goldChannels;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	unsigned char* goldData = stbi_load("src\\Models\\cyberrGold.png", &goldWidth, &goldHeight, &goldChannels, 0);
#else
    unsigned char* goldData = stbi_load("models/cyberrGold.png", &goldWidth, &goldHeight, &goldChannels, 0);
#endif
    if (goldData)
    {
        GLenum format = (goldChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, goldWidth, goldHeight, 0, format, GL_UNSIGNED_BYTE, goldData);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cerr << "Loaded cyberrGold.png" << std::endl;
    }
    else
    {
        std::cerr << "Failed to load cyberrGold.png" << std::endl;
    }
    stbi_image_free(goldData);

    // loads the wheels
    unsigned int tireTexture;
    glGenTextures(1, &tireTexture);
    glBindTexture(GL_TEXTURE_2D, tireTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int tireWidth, tireHeight, tireChannels;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	unsigned char* tireData = stbi_load("src\\Models\\wheelsBake.png", &tireWidth, &tireHeight, &tireChannels, 0);
#else
    unsigned char* tireData = stbi_load("models/wheelsBake.png", &tireWidth, &tireHeight, &tireChannels, 0);
#endif
    if (tireData)
    {
        GLenum format = (tireChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, tireWidth, tireHeight, 0, format, GL_UNSIGNED_BYTE, tireData);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cerr << "Loaded tireBake.png" << std::endl;
    }
    else
    {
        std::cerr << "Failed to load tireBake.png" << std::endl;
    }
    stbi_image_free(tireData);

    // loads the bumper and arch texture
    unsigned int bumperBorderTexture;
    glGenTextures(1, &bumperBorderTexture);
    glBindTexture(GL_TEXTURE_2D, bumperBorderTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int bbWidth, bbHeight, bbChannels;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	unsigned char* bbData = stbi_load("src\\Models\\bumperborderBake.png", &bbWidth, &bbHeight, &bbChannels, 0);
#else
    unsigned char* bbData = stbi_load("models/bumperborderBake.png", &bbWidth, &bbHeight, &bbChannels, 0);
#endif
    if (bbData)
    {
        GLenum format = (bbChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, bbWidth, bbHeight, 0, format, GL_UNSIGNED_BYTE, bbData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load bumperborderBake.png\n";
    }
    stbi_image_free(bbData);

    // Entering Main Loop
    while (!glfwWindowShouldClose(window))
    {
        // Each frame, reset color of each pixel to glClearColor
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Frame time calculation
        float currentFrameTime = glfwGetTime();
        float dt = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        carSpinAngle += carSpinSpeed * dt;
        wheelSpinAngle += wheelSpinSpeed * dt;

        double mousePosX, mousePosY;
        glfwGetCursorPos(window, &mousePosX, &mousePosY);

        //used for both the normal cam and the spin around obj
        if (!autoSpinMode) {

            double dx = mousePosX - lastMousePosX;
            double dy = mousePosY - lastMousePosY;
            lastMousePosX = mousePosX;
            lastMousePosY = mousePosY;
            
            const float cameraAngularSpeed = 8.0f;
            cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
            cameraVerticalAngle -= dy * cameraAngularSpeed * dt;
            cameraVerticalAngle = std::clamp(cameraVerticalAngle, -85.0f, 85.0f);

            float phi = glm::radians(cameraVerticalAngle);
            float theta = glm::radians(cameraHorizontalAngle);
            cameraLookAt = glm::vec3(cosf(phi) * cosf(theta), sinf(phi), -cosf(phi) * sinf(theta));

            processInput(window, dt, cameraPosition, cameraLookAt, cameraUp);
        }

        else {

            float actualSpinSpeed = spinSpeed;
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
                actualSpinSpeed *= 3.0f;
            }


            spinAngle += actualSpinSpeed * dt;

            float rad = glm::radians(spinAngle);
            glm::vec3 center(0.0f);
            float radius = 20.0f;
            float height = 15.0f;
            cameraPosition = center + glm::vec3(radius * cos(rad), height, radius * sin(rad));
            cameraLookAt = glm::normalize(center - cameraPosition);
        }


        // View Matrix
        glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);

        // Projection Matrix
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(70.0f), (float)WIDTH / (float)HEIGHT, 0.01f, 100.0f);




        // Animate directional light position in a circle
        dirLightAngle += dirLightSpeed * dt;
        if (dirLightAngle > 360.0f) dirLightAngle -= 360.0f;
        float rad = glm::radians(dirLightAngle);
        dirLightPos.x = dirLightRadius * cos(rad);
        dirLightPos.y = dirLightHeight;
        dirLightPos.z = dirLightRadius * sin(rad);

        //Directional light
        glm::vec3 lightPos = dirLightPos;
        glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 1.0f, 80.0f);
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        glm::mat4 floorWorld = glm::mat4(1.0f);
        floorWorld = glm::translate(floorWorld, glm::vec3(0.0f, -1.0f, 0.0f));
        floorWorld = glm::scale(floorWorld, glm::vec3(100.0f, 1.0f, 100.0f));
        floorWorld = glm::rotate(floorWorld, glm::radians(-90.0f), glm::vec3(1, 0, 0));

        glm::mat4 carWorld = glm::mat4(1.0f);
        carWorld = glm::translate(carWorld, glm::vec3(0.0f, -1.0f, 0.0f));
        carWorld = glm::rotate(carWorld, glm::radians(carSpinAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        carWorld = glm::scale(carWorld, glm::vec3(4.0f));

        // Shadow depth pass
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);

        glUseProgram(depthShader);
        glUniformMatrix4fv(glGetUniformLocation(depthShader, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

        // floor
        glUniformMatrix4fv(glGetUniformLocation(depthShader, "worldMatrix"), 1, GL_FALSE, &floorWorld[0][0]);
        glBindVertexArray(groundVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        //car and wheels mesh
        for (const Mesh& mesh : cubeMeshes) {
            glm::mat4 worldForThisMesh = carWorld;

            bool isWheel =
                (mesh.name.find("Tire") != std::string::npos) ||
                (mesh.name.find("Rims") != std::string::npos) ||
                (mesh.name.find("Protector") != std::string::npos);

            if (isWheel) {
                glm::vec3 pivot = meshCenter(mesh);
                glm::mat4 wheelLocal(1.0f);
                wheelLocal = glm::translate(wheelLocal, pivot);
                wheelLocal = glm::rotate(wheelLocal, glm::radians(wheelSpinAngle), glm::vec3(1, 0, 0));
                wheelLocal = glm::translate(wheelLocal, -pivot);
                worldForThisMesh = carWorld * wheelLocal;
            }

            glUniformMatrix4fv(glGetUniformLocation(depthShader, "worldMatrix"), 1, GL_FALSE, &worldForThisMesh[0][0]);
            mesh.Draw();
        }

        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, WIDTH, HEIGHT);


        //////// Skybox
        // Draw Skybox
        glDepthFunc(GL_LEQUAL);
        glUseProgram(skyboxShaderProgram);

        // View matrix Skybox
        glm::mat4 viewMatrixSkybox = glm::mat4(glm::mat3(viewMatrix));
        GLuint viewMatrixSkyboxLocation = glGetUniformLocation(skyboxShaderProgram, "viewMatrix");
        glUniformMatrix4fv(viewMatrixSkyboxLocation, 1, GL_FALSE, &viewMatrixSkybox[0][0]);

        GLuint projectionMatrixSkyboxLocation = glGetUniformLocation(skyboxShaderProgram, "projectionMatrix");
        glUniformMatrix4fv(projectionMatrixSkyboxLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // Reset depth function

        // draw fllor with shadow
        glUseProgram(floorShadowProgram);
        glUniformMatrix4fv(glGetUniformLocation(floorShadowProgram, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(floorShadowProgram, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(floorShadowProgram, "worldMatrix"), 1, GL_FALSE, &floorWorld[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(floorShadowProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture); // stone.jpg
        glUniform1i(glGetUniformLocation(floorShadowProgram, "diffuseTexture"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMapTex);
        glUniform1i(glGetUniformLocation(floorShadowProgram, "shadowMap"), 1);
        // draw fllor with shadow
        glUseProgram(floorShadowProgram);
        glUniformMatrix4fv(glGetUniformLocation(floorShadowProgram, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(floorShadowProgram, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(floorShadowProgram, "worldMatrix"), 1, GL_FALSE, &floorWorld[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(floorShadowProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture); // stone.jpg
        glUniform1i(glGetUniformLocation(floorShadowProgram, "diffuseTexture"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMapTex);
        glUniform1i(glGetUniformLocation(floorShadowProgram, "shadowMap"), 1);

        glBindVertexArray(groundVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);


        // draw models loaded using assimp
        glUseProgram(modelShaderProgram);

        // Set matrices for model shader
        GLuint modelViewLoc = glGetUniformLocation(modelShaderProgram, "viewMatrix");
        GLuint modelProjLoc = glGetUniformLocation(modelShaderProgram, "projectionMatrix");
        GLuint modelWorldLoc = glGetUniformLocation(modelShaderProgram, "worldMatrix");

        // pass light/shadow to model shader (same lightPos & lightSpaceMatrix you used for the floor)
        glUniformMatrix4fv(glGetUniformLocation(modelShaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);
        glUniform3fv(glGetUniformLocation(modelShaderProgram, "lightPosWorld"), 1, &lightPos[0]);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMapTex);
        glUniform1i(glGetUniformLocation(modelShaderProgram, "shadowMap"), 1);


        glUniformMatrix4fv(modelViewLoc, 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(modelProjLoc, 1, GL_FALSE, &projectionMatrix[0][0]);

        glUniformMatrix4fv(modelWorldLoc, 1, GL_FALSE, &carWorld[0][0]);

        GLuint colorLoc = glGetUniformLocation(modelShaderProgram, "modelColor");
        glUniform3fv(colorLoc, 1, &currentModelColor[0]);

        //finds center point of mesh so we can rotate it 

		std::vector<glm::vec3> wheelPositions;

        for (const Mesh& mesh : cubeMeshes) {

            glActiveTexture(GL_TEXTURE0);


            if (mesh.name.find("Tire") != std::string::npos ||
                mesh.name.find("Protector") != std::string::npos ||
                mesh.name.find("Rims") != std::string::npos)
            {
                glBindTexture(GL_TEXTURE_2D, tireTexture);
            }
            else if (mesh.name == "BackBumper" || mesh.name == "Arch" || mesh.name == "Arch.001" || mesh.name == "Border") {
                glBindTexture(GL_TEXTURE_2D, bumperBorderTexture);
            }
            else if (currentTextureMode == 1) {
                glBindTexture(GL_TEXTURE_2D, cyberTexture);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, goldTexture);
            }
			// Set material properties here
            glUniform1i(glGetUniformLocation(modelShaderProgram, "material.diffuse"), 0);
            if (mesh.name.find("Tire") != std::string::npos ||
                mesh.name.find("Protector") != std::string::npos ||
                mesh.name.find("Rims") != std::string::npos)
            {
                glBindTexture(GL_TEXTURE_2D, tireTexture);
            }
            else if (mesh.name == "BackBumper" || mesh.name == "Arch" || mesh.name == "Arch.001" || mesh.name == "Border") {
                glBindTexture(GL_TEXTURE_2D, bumperBorderTexture);
            }
            else if (currentTextureMode == 1) {
                glBindTexture(GL_TEXTURE_2D, cyberTexture);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, goldTexture);
            }
			// Set material properties here
            glUniform1i(glGetUniformLocation(modelShaderProgram, "material.diffuse"), 0);


            glm::mat4 worldForThisMesh = carWorld;

            // uses mesh names to find wheels and spin them on axis 
            bool isWheel = (mesh.name.find("Tire") != std::string::npos) || (mesh.name.find("Rims") != std::string::npos) || (mesh.name.find("Protector") != std::string::npos);
            // uses mesh names to find wheels and spin them on axis 
            bool isWheel = (mesh.name.find("Tire") != std::string::npos) || (mesh.name.find("Rims") != std::string::npos) || (mesh.name.find("Protector") != std::string::npos);

            if (isWheel) {
                glm::vec3 pivot = meshCenter(mesh);
                glm::vec3 axleAxis = glm::vec3(1.0f, 0.0f, 0.0f);
				wheelPositions.push_back(pivot);

                    glm::mat4 wheelLocal(1.0f);
                    wheelLocal = glm::translate(wheelLocal, pivot);

                    wheelLocal = glm::rotate(wheelLocal, glm::radians(wheelSpinAngle), axleAxis);
                    wheelLocal = glm::translate(wheelLocal, -pivot);

                worldForThisMesh = carWorld * wheelLocal;

                
            }

                glUniformMatrix4fv(modelWorldLoc, 1, GL_FALSE, &worldForThisMesh[0][0]);
                mesh.Draw();
            }
        for (int i = 0; i < wheelPositions.size(); i++)
        {
          
            particleSystems[i]->emitParticles(dt, cameraPosition);
            particleSystems[i]->update(dt);
            particleSystems[i]->render(viewMatrix, projectionMatrix);
        }



        // Rebind stone texture for floor
        glUseProgram(shaderProgram); // Use floor shader again
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture); // Rebind stone.jpg
        glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexture"), 0);

        angle += rotationSpeed * dt; // increment angle each frame
        if (angle > 360.0f)
            angle -= 360.0f;

        // Light cube
        // --- Light cube (shows point light position) ---
        for (unsigned int i = 0; i < 1; i++)
        {
            glUseProgram(lightCubeProgram);

            // Get uniform locations
            GLuint viewMatrixLocation = glGetUniformLocation(lightCubeProgram, "viewMatrix");
            GLuint projectionMatrixLocation = glGetUniformLocation(lightCubeProgram, "projectionMatrix");
            GLuint worldMatrixLocation = glGetUniformLocation(lightCubeProgram, "worldMatrix");

            // Send view & projection
            glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
            glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

            // Prepare cube VAO
            glBindVertexArray(cubeVAO);

            // Build transform for the light cube
            glm::mat4 worldMatrix = glm::mat4(1.0f);
            worldMatrix = glm::translate(worldMatrix, dirLightPos);
            worldMatrix = glm::scale(worldMatrix, glm::vec3(0.5f)); // make it smaller

            // Send world matrix
            glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);

            // Draw
            glDrawArrays(GL_TRIANGLES, 0, 36);

            glBindVertexArray(0);
        }


        {

            std::string title = std::string("Wrap: ") + (currentTextureMode == 1 ? "Onyx" : "Gold") + " | Mode: " + (autoSpinMode ? "Orbit" : "Manual") + " | F=Orbit 1/2=Wrap  Shift=Fast  Esc=Quit";
            glfwSetWindowTitle(window, title.c_str());
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Shutdown GLFW
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
