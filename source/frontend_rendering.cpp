#include "frontend.h"

#include <glm/gtc/type_ptr.hpp>

void SimulationFrontend::draw_tracers()
{
    if (render_tracers) {
        line_shader->use();
        line_shader->uniform_mat4("projection", glm::value_ptr(projection));
        line_shader->uniform_mat4("view", glm::value_ptr(view));

        line_vao->use();
        for (int i = 0; i < simulation.num_bodies; ++i) {
            auto& tracers = simulation.get_info(i).tracers;
            line_vbo->set_data(tracers, GL_DYNAMIC_DRAW);
            glDrawArrays(GL_LINES, 0, tracers.size());
        }
    }
}

void SimulationFrontend::draw_bodies()
{
    // Send the matrices to the GPU
    body_shader->use();
    body_shader->uniform_mat4("projection", glm::value_ptr(projection));
    body_shader->uniform_mat4("view",       glm::value_ptr(view));
    body_shader->uniform_vec3("camera_pos", glm::value_ptr(cam_pos));

    // Update the shader light sources
    static const auto set_light_param = [&](int num, std::string field, glm::vec3 value) {
        auto name = "lights[" + std::to_string(num) + "]." + field;
        body_shader->uniform_vec3(name.c_str(), glm::value_ptr(value));
    };

    int num_lights = 0;

    for (int i = 0; i < simulation.num_bodies; ++i) {
        const auto& instance = simulation.get_instance(i);
        const auto& physics  = simulation.get_physics(i);

        if (instance.emits_light) {
            set_light_param(num_lights, "pos",      physics.position);
            set_light_param(num_lights, "colour",   instance.colour);
            /*
            set_light_param(num_lights, "ambient",  instance.ambient);
            set_light_param(num_lights, "diffuse",  instance.diffuse);
            set_light_param(num_lights, "specular", instance.specular);
            */
            ++num_lights;
        }
    }

    body_shader->uniform_int("num_lights", num_lights);
    
    // Update the bodies vertex buffer
    bodies_instance_vbo->set_data(simulation.body_instance, GL_DYNAMIC_DRAW);
    
    body_vao->use();
    glDrawArraysInstanced(GL_TRIANGLES, 0, SPHERE_VERTEX_COUNT, simulation.num_bodies);
}

void SimulationFrontend::apply_bloom()
{
    constexpr int NUM_ITERATIONS = 3;
    bloom_shader->use();
    screen_vao->use();

    auto bloom_pass = [&](GLFrameBuffer *source, 
                          GLFrameBuffer *dest, 
                          bool horizontal, int tex) {
        bloom_shader->uniform_int("horizontal", horizontal);
        dest->use();
        source->use_texture(tex, GL_TEXTURE0);
        glDrawArrays(GL_TRIANGLES, 0, SCREEN_VERTEX_COUNT);
    };

    bloom_pass(hdr_fbo, bloom_vertical_fbo, false, 1);
    bloom_pass(bloom_vertical_fbo, bloom_horizontal_fbo, true, 0);

    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        bloom_pass(bloom_horizontal_fbo, bloom_vertical_fbo, false, 0);
        bloom_pass(bloom_vertical_fbo, bloom_horizontal_fbo, true, 0);
    }
}

void SimulationFrontend::render_scene()
{
    hdr_fbo->use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_tracers();
    draw_bodies();
    apply_bloom();

    use_default_framebuffer();
    glClear(GL_DEPTH_BUFFER_BIT);
    final_shader->use();
    hdr_fbo->use_texture(0, GL_TEXTURE0);
    bloom_horizontal_fbo->use_texture(0, GL_TEXTURE1);
    glDrawArrays(GL_TRIANGLES, 0, SCREEN_VERTEX_COUNT);
}
