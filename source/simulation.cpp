#include "simulation.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>

const BodyInfo& Simulation::get_info(int index) const
{
    static const BodyInfo dummy_info {
        "NONE", {}
    };

    return (index == NO_BODY)
        ? dummy_info
        : body_info[index];
}

const BodyPhysics& Simulation::get_physics(int index) const
{
    static const BodyPhysics dummy_physics;

    return (index == NO_BODY)
        ? dummy_physics
        : body_physics[index];
}

const BodyInstance& Simulation::get_instance(int index) const
{
    return body_instance[index];
}

void Simulation::clear_tracers()
{
    for (int i = 0; i < num_bodies; ++i) {
        auto& info = body_info[i];
        info.tracers.clear();
    }
}

void Simulation::delete_body(int index)
{
    if (index == draw_tracers_relative_to) {
        draw_tracers_relative_to = NO_BODY;
    }

    body_info.erase(body_info.begin() + index);
    body_physics.erase(body_physics.begin() + index);
    body_instance.erase(body_instance.begin() + index);
    --num_bodies;
}

void update_forces(std::vector<BodyPhysics>& bodies, float time_step)
{
    constexpr float epsilon = 0.0001;
    constexpr float grav_constant = 6.674e-3;

    int len = bodies.size();

    for (int i = 0; i < len; ++i) {
        auto& body = bodies[i];

        for (int j = 0; j < len; ++j) {
            // Check if this body and the other one are the same
            if (i == j) {
                continue;
            }

            const auto& other = bodies[j];
            glm::vec3 delta = other.position - body.position;
            glm::vec3 force_dir = glm::normalize(delta);
            float radius = glm::length(delta);

            // Avoid division by 0
            if (abs(radius) > epsilon) {
                float r2 = radius * radius;
                float a  = (grav_constant * other.mass) / r2;
                // F = Gm1m2/r^2, F = ma, a = Gm/r^2
                glm::vec3 acceleration = a * force_dir;
                body.velocity += acceleration * time_step;
            }
        }
    }
}

void Simulation::calculate_trajectories()
{
    constexpr int precalc_steps = 1000;
    constexpr int line_period = 10;

    // Create a copy of the physics to update for trajectories
    std::vector<BodyPhysics> copy;
    std::copy(body_physics.begin(), 
              body_physics.end(), 
              std::back_inserter(copy));

    for (int step = 0; step < precalc_steps; ++step) {
        update_forces(copy, 1.0f);
        for (int i = 0; i < num_bodies; ++i) {
            // Ignore if drawing relative to this body, as tracers
            // will all be the same as the body's position
            if (i == draw_tracers_relative_to) {
                continue;
            }

            auto& physics = copy[i];
            physics.position += physics.velocity;
            auto& info = body_info[i];
            
            // Trajectories may be drawn relative to a body,
            // so subtract it's position for all tracers
            glm::vec3 relative, relative_orig;

            if (draw_tracers_relative_to == Simulation::NO_BODY) {
                relative      = glm::vec3(0.0);
                relative_orig = glm::vec3(0.0);
            } else {
                relative      = copy[draw_tracers_relative_to].position;
                relative_orig = copy[draw_tracers_relative_to].orig_position;
            }
            
            if (step % line_period == 0) {
                auto tracer_pos = physics.position - (relative - relative_orig);
                info.tracers.push_back(tracer_pos);
            }
        }
    }    
}

void Simulation::update_positions()
{
    constexpr int trail_period = 10;
    auto relative_vel = get_physics(draw_tracers_relative_to).velocity;

    for (int i = 0; i < num_bodies; ++i) {
        auto& info     = body_info[i];
        auto& physics  = body_physics[i];
        auto& instance = body_instance[i];

        if (state == SimulationState::Running) {
            // Adjust the tracers to be in line with the relative body
            for (auto& tracer_vert : info.tracers) {
                tracer_vert += relative_vel;
            }

            if (num_updates % trail_period == 0) {
                info.tracers.push_back(physics.position);
            }

            physics.position += physics.velocity;
        } else if (state == SimulationState::Waiting) {
            // Clear the tracers and reset the velocity and position
            info.tracers.clear();
            physics.position = physics.orig_position;
            physics.velocity = physics.orig_velocity;
        }

        // Update the model matrix
        instance.model = glm::scale(
            glm::translate(glm::mat4(1.0f), physics.position),
            glm::vec3(physics.radius));
    }
}

void Simulation::update()
{
    // Update the physics 
    if (state == SimulationState::Running) {
        update_forces(body_physics, 1.0f);
    }

    update_positions();

    if (state == SimulationState::Waiting) {
        calculate_trajectories();
    }    

    ++num_updates;
}

void Simulation::load_simulation(const std::string &path)
{
    std::ifstream file(path, std::ios::binary);
    if (file.good()) {
        int count;
        file.read(reinterpret_cast<char*>(&count), sizeof(count));
        std::vector<BodyPhysics> phys;
        for (int _ = 0; _ < count; ++_) {
            BodyPhysics b;
            file.read(reinterpret_cast<char*>(&b), sizeof(b));
            phys.push_back(b);
        }
        std::vector<BodyInstance> inst;
        for (int _ = 0; _ < count; ++_) {
            BodyInstance i;
            file.read(reinterpret_cast<char*>(&i), sizeof(i));
            inst.push_back(i);
        }
        std::vector<BodyInfo> info;
        for (int _ = 0; _ < count; ++_) {
            size_t len;
            file.read(reinterpret_cast<char*>(&len), sizeof(len));
            std::string name = "";
            for (int __ = 0; __ < len; ++__) {
                char c;
                file.read(reinterpret_cast<char*>(&c), sizeof(c));
                name += c;
            }
            info.push_back(BodyInfo{name});
        }

        num_bodies = count;
        body_info = info;
        body_physics = phys;
        body_instance = inst;
    }
}

void Simulation::save_simulation(const std::string &path)
{
    std::ofstream file(path, std::ios::binary);
    if (file.good()) {
        file.write(reinterpret_cast<char*>(&num_bodies), sizeof(num_bodies));
        for (auto &physics : body_physics) {
            file.write(reinterpret_cast<char*>(&physics), sizeof(physics));
        }
        for (auto &instance : body_instance) {
            file.write(reinterpret_cast<char*>(&instance), sizeof(instance));
        }
        for (auto &info : body_info) {
            auto &name = info.name;
            auto len   = name.length();
            file.write(reinterpret_cast<char*>(&len), sizeof(len));
            file.write(reinterpret_cast<const char*>(name.c_str()), len);
        }
    }
}

