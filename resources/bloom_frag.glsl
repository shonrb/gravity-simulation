#version 330 core
out vec4 frag_colour;

in vec2 v_tex_coords;

uniform sampler2D image;
uniform bool horizontal;

// Bloom kernel
const float weight[5] = float[] (
    0.2270270270, 
    0.1945945946, 
    0.1216216216, 
    0.0540540541, 
    0.0162162162
);
const int NUM_PASSES = 5;

void main()
{             
    vec2 tex_offset = 1.0 / textureSize(image, 0);
    vec3 result = texture(image, v_tex_coords).rgb * weight[0];
    
    for (int i = 1; i < NUM_PASSES; ++i) {
        vec2 offset = (horizontal)
            ? vec2(tex_offset.x * i, 0.0)
            : vec2(0.0, tex_offset.y * i);

        result += texture(image, v_tex_coords + offset).rgb * weight[i];
        result += texture(image, v_tex_coords - offset).rgb * weight[i];
    }

    frag_colour = vec4(result, 1.0);
}

