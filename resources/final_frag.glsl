#version 330 core
out vec4 frag_colour;

in vec2 v_tex_coords;

uniform sampler2D scene;
uniform sampler2D bloom;
const float exposure = 1.0;
const float gamma = 2.2;

void main()
{             
    // Gamma correction
    vec3 res = texture(scene, v_tex_coords).rgb + texture(bloom, v_tex_coords).rgb;
    res = vec3(1.0) - exp(-res * exposure);
    res = pow(res, vec3(1.0 / gamma));
    frag_colour = vec4(res, 1.0);
}