#pragma once

#define GLM_FORCE_SWIZZLE
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include <iostream>
#include <array>
#include <vector>
#include <optional>
#include <memory>

#include "shader.h"
#include "gl_objects.h"
#include "simulation.h"

constexpr std::array SPHERE_MESH = {
#include "../resources/spheremesh.txt"
};

constexpr std::array SCREEN_MESH = {
//   Position       Tex coords
    -1.0f,  1.0f,   0.0f, 1.0f,
    -1.0f, -1.0f,   0.0f, 0.0f,
     1.0f, -1.0f,   1.0f, 0.0f,
    -1.0f,  1.0f,   0.0f, 1.0f,
     1.0f, -1.0f,   1.0f, 0.0f,
     1.0f,  1.0f,   1.0f, 1.0f
};

constexpr unsigned SPHERE_VERTEX_COUNT = SPHERE_MESH.size() / 3;

constexpr unsigned SCREEN_VERTEX_COUNT = SCREEN_MESH.size() / 4;

class SimulationFrontend {
    // Windowing
    int window_width  = 1000;
    int window_height = 1000;
    SDL_Window *window;
    SDL_GLContext context;

    // Rendering
    GLFrameBuffer *hdr_fbo;
    GLFrameBuffer *bloom_horizontal_fbo;
    GLFrameBuffer *bloom_vertical_fbo;

    GLVertexBuffer *line_vbo;
    GLVertexBuffer *bodies_instance_vbo;
    GLVertexBuffer *sphere_vbo;
    GLVertexBuffer *screen_vbo;

    GLVertexArray *body_vao;
    GLVertexArray *line_vao;
    GLVertexArray *screen_vao;

    Shader *line_shader;
    Shader *body_shader;
    Shader *bloom_shader;
    Shader *final_shader;

    glm::mat4 projection;
    glm::mat4 view;

    int tracked_body = Simulation::NO_BODY;
    bool render_tracers = true;

    // Camera 
    float cam_dist = 20.0f;
    float cam_angle_x = 0.0f;
    float cam_angle_y = 90.0f;
    glm::vec3 cam_pos;

    // Input
    bool running = true;
    bool holding = false;
    int prev_mouse_x = 0;
    int prev_mouse_y = 0;
    int mouse_held_time = 0;
    bool mouse_clicked = false;

    // Simulation
    Simulation simulation;

    // UI
    BodyPhysics  prototype_physics;
    BodyInfo     prototype_info { "Body", {} };
    BodyInstance prototype_instance {
        glm::mat4(1.0f),
        glm::vec3(1.0f, 0.5f, 0.31f),
        0
    };

public:
    SimulationFrontend();
    ~SimulationFrontend();
    void run();

private:
    // Initialisation
    void init_graphics();
    void init_framebuffers();
    void init_vertex_buffers();
    void init_vertex_arrays();
    void init_shaders();

    // Updating
    void handle_events();
    void handle_mouse_input();
    void update_viewport();
    void update_camera();

    // Drawing
    void draw_tracers();
    void draw_bodies();
    void apply_bloom();
    void render_scene();

    // GUI
    void ui_state_switching();
    void ui_body_selection();
    void ui_state_specifics();
    void ui_saving_loading();
    void show_ui();
};

