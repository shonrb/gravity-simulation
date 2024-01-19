#include "gl_objects.h"
#include <map>
#include <cstdio>
#include <cassert>
#include <iostream>

// GLFrameBuffer

GLFrameBuffer::GLFrameBuffer(int w, int h)
: width(w), height(h)
{
    glGenFramebuffers(1, &handle);
    use();
}

GLFrameBuffer::~GLFrameBuffer()
{
    glDeleteFramebuffers(1, &handle);
}

void GLFrameBuffer::attach_textures(int count)
{
    use();
    std::vector<unsigned> attachments;

    for (int i = 0; i < count; ++i) {
        unsigned texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, 
            width, height, 0, 
            GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, 
            GL_COLOR_ATTACHMENT0 + i, 
            GL_TEXTURE_2D,
            texture, 
            0);
        textures.push_back(texture);
        attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
    }

    if (count > 1)
        glDrawBuffers(count, attachments.data());
}

void GLFrameBuffer::attach_renderbuffer()
{
    // Attach a renderbuffer to the fbo. This allows 
    // depth testing without the need for a separate texture
    use();
    glGenRenderbuffers(1, &renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(
        GL_RENDERBUFFER, 
        GL_DEPTH_COMPONENT, 
        width, 
        height);
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, 
        GL_DEPTH_ATTACHMENT, 
        GL_RENDERBUFFER, 
        renderbuffer);
}

void GLFrameBuffer::use_texture(int index, unsigned texture_index) const
{
    glActiveTexture(texture_index);
    glBindTexture(GL_TEXTURE_2D, textures[index]);
    glActiveTexture(GL_TEXTURE0);
}

void GLFrameBuffer::use() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
}


void GLFrameBuffer::check() const
{
    use();
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer initialisation failed\n";
    }
}

void use_default_framebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// GLVertexBuffer

GLVertexBuffer::GLVertexBuffer() 
{
    glGenBuffers(1, &handle);
}

GLVertexBuffer::~GLVertexBuffer()
{
    glDeleteBuffers(1, &handle);
}

void GLVertexBuffer::use() const
{
    glBindBuffer(GL_ARRAY_BUFFER, handle);
}

// GLVertexArray

GLVertexArray::GLVertexArray() 
{
    glGenVertexArrays(1, &handle);
}

GLVertexArray::~GLVertexArray()
{
    glDeleteVertexArrays(1, &handle);
}

void GLVertexArray::use() const
{
    glBindVertexArray(handle);
}

