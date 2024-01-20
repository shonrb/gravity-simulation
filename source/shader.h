#ifndef __SHADER_H
#define __SHADER_H

#include <string>
#include <iostream>

class Shader {
    unsigned handle;
    unsigned vert_handle;
    unsigned frag_handle;

public:
    Shader(const char *vert_path, const char *frag_path);
    ~Shader();
    void uniform_int(const char *name, int value) const;
    void uniform_float(const char *name, float value) const;
    void uniform_vec2(const char *name, const float *value) const;
    void uniform_vec3(const char *name, const float *value) const;
    void uniform_vec4(const char *name, const float *value) const;
    void uniform_mat4(const char* name, float* value) const;
    void use() const;

private:
    void load_and_compile(const char *path, unsigned shader_handle) const;
    void compile_program(unsigned v_handle, unsigned f_handle);
    void log_error(int error_type, unsigned shader, const char *target) const;
};
#endif

