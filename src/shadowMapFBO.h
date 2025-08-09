#pragma once
#define GLEW_STATIC 1 // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>  // Include GLEW - OpenGL Extension Wrangler
#include <iostream>

class ShadowMapFBO
{
public:
    GLuint m_fbo;
    ShadowMapFBO()
    {
        m_fbo = 0;
        m_shadowMap = 0;
        m_width = 0;  // Default width
        m_height = 0; // Default height
    }

    ~ShadowMapFBO()
    {
        if (m_shadowMap)
        {
            glDeleteTextures(1, &m_shadowMap);
        }
        if (m_fbo)
        {
            glDeleteFramebuffers(1, &m_fbo);
        }
    }

    bool init(unsigned int width, unsigned int height);
    void bindForWriting();
    void bindForReading(GLenum textureUnit);

private:
    unsigned int m_width, m_height = 0;
    GLuint m_shadowMap;
};