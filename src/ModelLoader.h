#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <vector>
#include <string>
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Mesh {
    unsigned int VAO, VBO, EBO;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int textureID =0;
    std::string name;

    void setupMesh();
    void Draw() const;
};

void processMesh(aiMesh* mesh, std::vector<Mesh>& meshes);
void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes);
std::vector<Mesh> loadModel(const std::string& path);

#endif
