#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <string>

struct BodyInfo {
    std::string name;
    std::vector<glm::vec3> tracers;
};

struct BodyPhysics {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 orig_position = glm::vec3(0.0f);
    glm::vec3 orig_velocity = glm::vec3(0.0f);
    float mass         = 1.0f;
    float radius       = 1.0f;
};

struct BodyInstance {
    glm::mat4 model; 
    glm::vec3 colour;
    int emits_light;
};

enum class SimulationState {
    Waiting, Running, Paused
};

struct Simulation {
    static constexpr int NO_BODY = -1;
    int num_updates = 0;
    int num_bodies = 0;
    std::vector<BodyInfo>     body_info;
    std::vector<BodyPhysics>  body_physics;
    std::vector<BodyInstance> body_instance;
    SimulationState state = SimulationState::Waiting;
    int draw_tracers_relative_to = NO_BODY;

    const BodyInfo& get_info(int index) const;
    const BodyPhysics& get_physics(int index) const;
    const BodyInstance& get_instance(int index) const;
    void clear_tracers();
    void delete_body(int index);
    void update();

private:
    void calculate_trajectories();
    void update_positions();
};