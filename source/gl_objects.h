#ifndef __GLOBJECTS
#define __GLOBJECTS

#include <GL/glew.h>
#include <map>
#include <vector>
#include <iostream>

class GLFrameBuffer {
    unsigned handle;
    unsigned renderbuffer;
    int width;
    int height;
    std::vector<unsigned> textures;

public:
    GLFrameBuffer(int w, int h);
    ~GLFrameBuffer();
    void attach_textures(int count);
    void attach_renderbuffer();
    void use_texture(int index, unsigned texture_index) const;
    void use() const;
    void check() const;
};

void use_default_framebuffer();

class GLVertexBuffer {
    unsigned handle;

public:
    GLVertexBuffer();
    ~GLVertexBuffer();
    template<typename T> 
    void set_data(const typename std::vector<T>& buffer, GLenum usage);
    template<typename T, size_t N> 
    void set_data(const typename std::array<T, N>& buffer, GLenum usage);
    void use() const;
};

template<typename T> 
void GLVertexBuffer::set_data(const typename std::vector<T>& buffer, GLenum usage)
{
    use();
    size_t size = sizeof(T) * buffer.size();
    glBufferData(GL_ARRAY_BUFFER, size, buffer.data(), usage);
}

template<typename T, size_t N> 
void GLVertexBuffer::set_data(const typename std::array<T, N>& buffer, GLenum usage)
{
    use();
    size_t size = sizeof(T) * buffer.size();
    glBufferData(GL_ARRAY_BUFFER, size, buffer.data(), usage);
}

class GLVertexArray {
    unsigned handle;
    size_t count = 0;

public:
    enum Type {
        Attr, 
        Padding, 
        InstanceAttr
    };

    GLVertexArray();
    ~GLVertexArray();
    void use() const;

    template<typename ...T>
    void add_attrs(GLVertexBuffer& buffer, T... data_args);
};

enum class VAOType {
    Attr, Padding, InstanceAttr
};  

struct VertexData {
    size_t size;
    GLenum datatype;
    GLVertexArray::Type type;
};

template<typename ...T>
void GLVertexArray::add_attrs(GLVertexBuffer& buffer, T... data_args)
{
    static std::map<GLenum, size_t> TYPE_TO_SIZE = {
        { GL_BYTE,           1 },
        { GL_UNSIGNED_BYTE,  1 },
        { GL_SHORT,          2 },
        { GL_UNSIGNED_SHORT, 2 },
        { GL_INT,            4 },
        { GL_UNSIGNED_INT,   4 },
        { GL_FLOAT,          4 },
        { GL_DOUBLE,         8 },
    };
    
    std::vector data{data_args...};
    buffer.use();
    use();

    size_t stride = 0;
    for (auto& space : data) 
        stride += space.size * TYPE_TO_SIZE[space.datatype];
    
    size_t offset = 0;

    for (auto& space : data) {
        if (space.type != GLVertexArray::Padding) {
            glVertexAttribPointer(
                count,
                space.size,
                space.datatype,
                false,
                stride,
                (void*) offset
            );
            if (space.type == GLVertexArray::InstanceAttr) 
                glVertexAttribDivisor(count, 1);
            glEnableVertexAttribArray(count);
            count++;
        }
        offset += space.size * TYPE_TO_SIZE[space.datatype];
    }
}



#endif
