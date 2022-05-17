#include "frontend.h"

#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <misc/cpp/imgui_stdlib.h>

std::vector<float> SPHERE_MESH = {
#include "../resources/spheremesh.txt"
};

std::vector<float> SCREEN_MESH = {
//   Position       Tex coords
    -1.0f,  1.0f,   0.0f, 1.0f,
    -1.0f, -1.0f,   0.0f, 0.0f,
     1.0f, -1.0f,   1.0f, 0.0f,
    -1.0f,  1.0f,   0.0f, 1.0f,
     1.0f, -1.0f,   1.0f, 0.0f,
     1.0f,  1.0f,   1.0f, 1.0f
};

void SimulationFrontend::init_graphics()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cout << "Error: " << SDL_GetError() << "\n";
    }

    // OpenGL config
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetSwapInterval(1);

    // Setup imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Windowing
    window = SDL_CreateWindow(
        "Gravity Simulation",
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        window_width, 
        window_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);
    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 130");

    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialisation failed\n";
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); 
}

void SimulationFrontend::init_framebuffers()
{
    hdr_fbo              = new GLFrameBuffer(window_width, window_height);
    bloom_horizontal_fbo = new GLFrameBuffer(window_width, window_height);
    bloom_vertical_fbo   = new GLFrameBuffer(window_width, window_height);

    // HDR rendering framebuffer: one texture for regular rendering, one texture
    // for light emitters and a renderbuffer for the depth buffer
    hdr_fbo->attach_textures(2);
    hdr_fbo->attach_renderbuffer();
    hdr_fbo->check();
    
    // Bloom framebuffers: Each has one texture for 
    // blurring the light emitter texture from hdr_fbo. 
    bloom_horizontal_fbo->attach_textures(1);
    bloom_horizontal_fbo->check();

    bloom_vertical_fbo->attach_textures(1);
    bloom_horizontal_fbo->check();
}

void SimulationFrontend::init_vertex_buffers()
{
    line_vbo            = new GLVertexBuffer();
    bodies_instance_vbo = new GLVertexBuffer();
    sphere_vbo          = new GLVertexBuffer();
    screen_vbo          = new GLVertexBuffer();

    // Set the buffer data that doesn't change dynamically
    sphere_vbo->set_data(SPHERE_MESH, GL_STATIC_DRAW);
    screen_vbo->set_data(SCREEN_MESH, GL_STATIC_DRAW);
}

void SimulationFrontend::init_vertex_arrays()
{
    body_vao   = new GLVertexArray();
    line_vao   = new GLVertexArray();
    screen_vao = new GLVertexArray();

    // Sphere mesh: only a position vec3 per vertex
    body_vao->add_attrs(
        *sphere_vbo,
        VertexData{ 3, GL_FLOAT, GLVertexArray::Attr }
    );

    // Body instances: a model mat4, a colour v3 and a light boolean 
    body_vao->add_attrs(
        *bodies_instance_vbo,
        VertexData{ 4, GL_FLOAT, GLVertexArray::InstanceAttr },  // Model
        VertexData{ 4, GL_FLOAT, GLVertexArray::InstanceAttr }, 
        VertexData{ 4, GL_FLOAT, GLVertexArray::InstanceAttr },
        VertexData{ 4, GL_FLOAT, GLVertexArray::InstanceAttr },
        VertexData{ 3, GL_FLOAT, GLVertexArray::InstanceAttr },  // Colour
        VertexData{ 1, GL_INT,   GLVertexArray::InstanceAttr }   // Emits light
    );

    // Tracer lines: only a position v3 per vertex
    line_vao->add_attrs(
        *line_vbo,
        VertexData{ 3, GL_FLOAT, GLVertexArray::Attr }
    );

    // Screen: a position vec2 and texcoord vec2 per vertex
    screen_vao->add_attrs(
        *screen_vbo,
        VertexData{ 2, GL_FLOAT, GLVertexArray::Attr }, // position
        VertexData{ 2, GL_FLOAT, GLVertexArray::Attr }  // texcoords
    );
}

void SimulationFrontend::init_shaders()
{
    line_shader  = new Shader("resources/line_vert.glsl",  
                              "resources/line_frag.glsl");
    body_shader  = new Shader("resources/body_vert.glsl",  
                              "resources/body_frag.glsl");
    bloom_shader = new Shader("resources/frame_vert.glsl", 
                              "resources/bloom_frag.glsl");
    final_shader = new Shader("resources/frame_vert.glsl", 
                              "resources/final_frag.glsl");

    // Initialise the texture uniforms
    bloom_shader->use();
    bloom_shader->uniform_int("image", 0);

    final_shader->use();
    final_shader->uniform_int("scene", 0);
    final_shader->uniform_int("bloom", 1);
}

SimulationFrontend::SimulationFrontend()
: SPHERE_VERTEX_COUNT(SPHERE_MESH.size() / 3)
, SCREEN_VERTEX_COUNT(SCREEN_MESH.size() / 4)
{
    init_graphics();
    init_framebuffers();
    init_vertex_buffers();
    init_vertex_arrays();
    init_shaders();

    update_viewport();
    update_camera();
}

void SimulationFrontend::handle_events()
{   
    constexpr int MAX_CLICK_TIME = 10;
    constexpr float SCROLL_COEFF = 0.3f;

    // If the mouse was clicked last frame, it isn't this frame
    mouse_clicked = false;

    // Handle events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type) {
        case SDL_QUIT:
            running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
            break;
        case SDL_MOUSEWHEEL:
            cam_dist += (float) event.wheel.y * SCROLL_COEFF;
            break;
        case SDL_MOUSEBUTTONDOWN:
            holding = true;
            break;
        case SDL_MOUSEBUTTONUP:
            holding = false;
            // If the mouse was held for few 
            // enough frames, treat it as a click
            if (mouse_held_time < MAX_CLICK_TIME)
                mouse_clicked = true;
            mouse_held_time = 0;
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                // Update the window dimensions, as well as everything
                // that varies with them.
                window_width = event.window.data1;
                window_height = event.window.data2;
                update_viewport();
                delete hdr_fbo;
                delete bloom_horizontal_fbo;
                delete bloom_vertical_fbo;
                init_framebuffers();
            }
            break;
        }
    }
}

void SimulationFrontend::handle_mouse_input()
{
    // Handle mouse movement and dragging
    constexpr int CAMERA_ROT_COEFF = 1.0f;
    constexpr float MAX_X_ANGLE = 89.0f;
    constexpr float MIN_X_ANGLE = -89.0f;

    ImGuiIO& io = ImGui::GetIO();
    int mouse_x;
    int mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    if (holding) {
        ++mouse_held_time;

        // If the mouse is not on the UI, rotate the 
        // camera based on the mouse dragging
        if (!io.WantCaptureMouse) {
            cam_angle_y += (mouse_x - prev_mouse_x) * CAMERA_ROT_COEFF;
            cam_angle_x += (mouse_y - prev_mouse_y) * CAMERA_ROT_COEFF;
            cam_angle_x = std::clamp(cam_angle_x, MIN_X_ANGLE, MAX_X_ANGLE);
        }
    }
    prev_mouse_x = mouse_x;
    prev_mouse_y = mouse_y;

    if (mouse_clicked && !io.WantCaptureMouse) {
        // Begin tracking the body behind the mouse cursor.
        // Convert the mouse position to NDC
        float x = (2.0f * mouse_x) / window_width - 1.0f;
        float y = 1.0f - (2.0f * mouse_y) / window_height;

        // Un-apply the transformation matrices 
        // to get the mouse position in world space
        auto click_pos_clip     = glm::vec4(x, y, -1.0f, 1.0f);
        auto click_pos_eye      = glm::inverse(projection) * click_pos_clip;
        auto click_pos_eye_back = glm::vec4(click_pos_eye.xy(), -1.0f, 0.0f);
        auto click_pos_world    = glm::inverse(view) * click_pos_eye_back;

        // Normalising will give us a ray towards the pixel
        float dist = INFINITY;
        auto ray = glm::normalize(click_pos_world.xyz());
        tracked_body = Simulation::NO_BODY;

        for (int i = 0; i < simulation.num_bodies; ++i) {
            auto pos    = simulation.get_physics(i).position;
            auto radius = simulation.get_physics(i).radius;

            // Compute the ray-sphere intersection
            auto a = (cam_pos - pos);
            auto b = glm::dot(a, ray);
            auto c = glm::dot(a, a) - radius * radius;
            auto d = glm::length(a);

            if (b * b - c >= 0 && d < dist) {
                tracked_body = i;
                dist = d;
            }
        }
    }
}

void SimulationFrontend::update_camera()
{
    constexpr glm::vec3 UP(0.0f, 1.0f, 0.0f);
    constexpr float MIN_DIST_FROM_BODY = 1.0f;

    const auto& body = simulation.get_physics(tracked_body);
    auto tracked_pos    = body.position;
    auto tracked_radius = body.radius;

    // Prevent the camera from going inside the body
    cam_dist = std::max(tracked_radius + MIN_DIST_FROM_BODY, cam_dist);
    
    // Get the vector from the body to the camera
    auto cam_dir = glm::normalize(glm::vec3(
        cos(glm::radians(cam_angle_y)) * cos(glm::radians(cam_angle_x)),
        sin(glm::radians(cam_angle_x)),
        sin(glm::radians(cam_angle_y)) * cos(glm::radians(cam_angle_x))
    ));

    // Calculate the camera position and view matrix
    cam_pos = tracked_pos + cam_dir * cam_dist;
    view = glm::lookAt(cam_pos, tracked_pos, UP);
}

void SimulationFrontend::update_viewport()
{
    constexpr float FOV       = glm::radians(45.0f);
    constexpr float NEAR_CLIP = .1f;
    constexpr float FAR_CLIP  = 1000.0f;

    float aspect_ratio = (float) window_width / (float) window_height;

    // Calculate the projection matrix and update the viewport
    projection = glm::perspective(FOV, aspect_ratio, NEAR_CLIP, FAR_CLIP);
    glViewport(0, 0, window_width, window_height);
}

void SimulationFrontend::run()
{
    // Main loop
    while (running) {
        handle_events();
        handle_mouse_input();

        simulation.update();

        update_camera();
        render_scene();
        show_ui();

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

SimulationFrontend::~SimulationFrontend()
{
    delete hdr_fbo;
    delete bloom_horizontal_fbo;
    delete bloom_vertical_fbo;
    delete line_vbo;
    delete bodies_instance_vbo;
    delete sphere_vbo;
    delete screen_vbo;
    delete body_vao;
    delete line_vao;
    delete screen_vao;
    delete line_shader;
    delete body_shader;
    delete bloom_shader;
    delete final_shader;  

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit(); 
}