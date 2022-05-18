#version 330 core 
layout (location = 0) out vec4 frag_colour;
layout (location = 1) out vec4 emitter_colour;

struct LightSource {
    vec3 pos;
    vec3 colour;
};

uniform LightSource lights[255];
uniform int         num_lights;
uniform vec3        camera_pos;

in      vec3  v_pos;
in      vec3  v_colour;
in      vec3  v_frag_pos;
flat in int   v_is_light;

float ambient_strength  = 0.2;
float diffuse_strength  = 0.6;
float specular_strength = 0.8;
float shininess         = 32.0;
vec3  dir_light_dir     = vec3(1.0, 1.0, 1.0);
vec3  dir_light_colour  = vec3(0.5, 0.5, 0.5);

vec3 calculate_light(vec3 light_dir, 
                     vec3 light_colour)
{
    light_dir = normalize(light_dir);
    vec3 normal = v_pos; // On a unit sphere the normal is equal to the pos
    
    // Diffuse
    float diffuse_mul = max(dot(normal, light_dir), 0.0);
    vec3  diffuse     = light_colour * diffuse_mul * v_colour * diffuse_strength;

    // Specular
    vec3  view_dir     = normalize(camera_pos - v_frag_pos);
    vec3  reflect_dir  = reflect(-light_dir, normal);
    float specular_mul = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
    vec3  specular     = light_colour * specular_mul * v_colour * specular_strength;

    return diffuse + specular;
}

vec3 reflector_body()
{
    vec3 result = calculate_light(dir_light_dir, dir_light_colour);

    // Add ambient light
    result += dir_light_colour * v_colour * ambient_strength;

    for (int i = 0; i < num_lights; ++i) {
        LightSource light     = lights[i];
        vec3        direction = light.pos - v_frag_pos;

        result += calculate_light(direction, light.colour);
    }

    return result;
}

void main()
{
    if (v_is_light > 0) {
        frag_colour    = vec4(v_colour, 1.0);
        emitter_colour = frag_colour;
    } else {
        frag_colour    = vec4(reflector_body(), 1.0);
        emitter_colour = vec4(0.0, 0.0, 0.0, 1.0);
    }
}