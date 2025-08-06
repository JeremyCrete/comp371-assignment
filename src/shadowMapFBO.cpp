#include "shadowMapFBO.h"

bool shadowMapFBO::init(unsigned int width, unsigned int height)
{
    m_width = width;
    m_height = height;

    glGenTextures(1, &m_shadowMap);
    glBindTextures(GL_TEXTURE_2D, m_shadowMap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTextParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindFrameBuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTCHEMENT, GL_TEXTURE_2D, m_shadowMap, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status !_ GL_FRAMEBUFFER_COMPLETE)
    {
        printf("FB error, status") return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

void ShadowMapFBO::bindForWriting()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
    glViewport(0, 0, m_width, m_height);
}

void ShadowMapFBO::bindForReading(GLenum textureUnit)
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
}