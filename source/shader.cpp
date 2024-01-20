#include "shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>

#include <GL/glew.h>

static void check_shader_errors(
    unsigned    shader_handle,
    const char *target,
    auto      &&get_iv,
    auto      &&get_log,
    int         status) 
{
    constexpr int BUF_SIZE = 1024;
    char info[BUF_SIZE];

    int success;
    get_iv(shader_handle, status, &success);

    if (!success) {
        get_log(shader_handle, BUF_SIZE, nullptr, info);
        std::cout << "Error in " << target << ":\n" << info << "\n";
    }
}


Shader::Shader(const char *vert_path, const char *frag_path)
{
    vert_handle = glCreateShader(GL_VERTEX_SHADER);
    frag_handle = glCreateShader(GL_FRAGMENT_SHADER);

    load_and_compile(vert_path, vert_handle);
    load_and_compile(frag_path, frag_handle);

    compile_program(vert_handle, frag_handle);
}

Shader::~Shader()
{
    glDeleteShader(vert_handle);
    glDeleteShader(frag_handle);
    glDeleteProgram(handle);
}

void Shader::load_and_compile(const char *path, unsigned shader_handle) const
{
    std::ifstream ifs(path);
    std::string src(std::istreambuf_iterator<char>{ifs}, {});
    const char *csrc = src.c_str();

    glShaderSource(shader_handle, 1, &csrc, NULL);
    glCompileShader(shader_handle);

    check_shader_errors(
        shader_handle, 
        path, 
        glGetShaderiv, 
        glGetShaderInfoLog, 
        GL_COMPILE_STATUS);
}

void Shader::compile_program(unsigned v_handle, unsigned f_handle)
{
    handle = glCreateProgram();
    glAttachShader(handle, v_handle);
    glAttachShader(handle, f_handle);
    glLinkProgram(handle);

    check_shader_errors(
        handle, 
        "linker", 
        glGetProgramiv, 
        glGetProgramInfoLog, 
        GL_LINK_STATUS);
}

void Shader::use() const 
{
    glUseProgram(handle);
}

void Shader::uniform_int(const char* name, int value) const
{
    glUniform1i(glGetUniformLocation(handle, name), value);
}

void Shader::uniform_float(const char* name, float value) const
{
    glUniform1f(glGetUniformLocation(handle, name), value);
}

void Shader::uniform_vec2(const char* name, const float* value) const
{
    glUniform2fv(glGetUniformLocation(handle, name), 1, value);
}

void Shader::uniform_vec3(const char* name, const float* value) const
{
    glUniform3fv(glGetUniformLocation(handle, name), 1, value);
}

void Shader::uniform_vec4(const char* name, const float* value) const
{
    glUniform4fv(glGetUniformLocation(handle, name), 1, value);
}

void Shader::uniform_mat4(const char* name, float* value) const
{
    int location = glGetUniformLocation(handle, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

