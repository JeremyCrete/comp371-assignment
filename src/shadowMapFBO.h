#pragma once
#define GLEW_STATIC 1 // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>  // Include GLEW - OpenGL Extension Wrangler

class ShadowMapFBO
{
public:
    ShadowMapFBO();
    ~ShadowMapFBO();

    bool init(unsigned int width, unsigned int height);
    void bindForWriting();
    void bindForReading(GLenum textureUnit);

private:
    unsigned int m_width, m_height = 0;
    GLuint m_id;
    GLuint m_shadowMap;
};