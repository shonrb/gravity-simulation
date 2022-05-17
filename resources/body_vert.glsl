#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in mat4 a_model;
layout (location = 5) in vec3 a_colour;
layout (location = 6) in int  a_is_light;

uniform mat4 view;
uniform mat4 projection;

out      vec3  v_pos;
out      vec3  v_colour;
flat out int   v_is_light;
out      vec3  v_frag_pos;

void main() 
{
    gl_Position = projection * view * a_model * vec4(a_pos, 1.0);
    v_pos      = a_pos;
    v_colour   = a_colour;
    v_is_light = a_is_light;
    v_frag_pos = vec3(a_model * vec4(a_pos, 1.0));
}