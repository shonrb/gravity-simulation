#include "shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>

#include <GL/glew.h>

enum ShaderStage {
    COMPILATION = 0,
    LINKING = 1
};

Shader::Shader(const char *vert_path, const char *frag_path)
{
    vert_handle = glCreateShader(GL_VERTEX_SHADER);
    frag_handle = glCreateShader(GL_FRAGMENT_SHADER);

    load_and_compile(vert_path, vert_handle);
    load_and_compile(frag_path, frag_handle);

    compile_program(vert_handle, frag_handle);
    use();
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

    check_for_errors(COMPILATION, shader_handle, path);
}

void Shader::compile_program(unsigned v_handle, unsigned f_handle)
{
    handle = glCreateProgram();
    glAttachShader(handle, v_handle);
    glAttachShader(handle, f_handle);
    glLinkProgram(handle);

    check_for_errors(LINKING, handle, "linker");
}

void Shader::check_for_errors(int stage, 
                              unsigned shader_handle, 
                              const char *target) const
{
    // Mappings from ShaderStage to opengl functions
    static const decltype(glGetShaderiv)      get_iv[]  = { glGetShaderiv, 
                                                            glGetProgramiv };
    static const decltype(glGetShaderInfoLog) get_log[] = { glGetShaderInfoLog, 
                                                            glGetProgramInfoLog };
    static const int                          status[]  = { GL_COMPILE_STATUS,  
                                                            GL_LINK_STATUS };

    constexpr int BUF_SIZE = 1024;
    char info[BUF_SIZE];

    int success;
    get_iv[stage](shader_handle, status[stage], &success);

    if (!success) {
        get_log[stage](shader_handle, BUF_SIZE, nullptr, info);
        std::cout << "Error in " << target << ":\n" << info << "\n"; 
    }
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

