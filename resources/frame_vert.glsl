#version 330 core
layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_tex_coords;

out vec2 v_tex_coords;

void main()
{
    v_tex_coords = a_tex_coords;
    gl_Position = vec4(a_position.xy, 0.0, 1.0);
}