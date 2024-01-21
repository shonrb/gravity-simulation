/* frontend_gui.cpp
 * 
 */
#include "frontend.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <misc/cpp/imgui_stdlib.h>

static void ui_body_editing(BodyInfo&     info, 
                            BodyPhysics&  physics, 
                            BodyInstance& instance)
{
    static const float STEP = 0.01f;
    static const float MIN = 0.01f;

    int input_number = 0;

    const auto scalar_input = [&](std::string label, float *ptr) {
        // Anything after the "##" will not be displayed.
        // This helps since IDs need to be unique.
        std::string id = label + "##" + std::to_string(input_number);
        const char *cid = id.c_str();
        ImGui::InputScalar(cid, ImGuiDataType_Float, ptr, &STEP);
        input_number++;
    };

    const auto vector3_input = [&](
        const char *name, 
        const char *comp0, 
        const char *comp1, 
        const char *comp2, 
        glm::vec3 *v) {

        ImGui::Text(name);
        scalar_input(comp0, &v->x);
        scalar_input(comp1, &v->y);
        scalar_input(comp2, &v->z);
    };

    ImGui::InputText("name", &info.name);

    // Physics options
    if (ImGui::CollapsingHeader("Physics")) {
        scalar_input("mass", &physics.mass);
        scalar_input("radius", &physics.radius);

        // Prevent the radius and mass from becoming 0
        physics.mass   = std::max(physics.mass,   MIN);
        physics.radius = std::max(physics.radius, MIN);

        vector3_input("Position", "x", "y", "z", &physics.orig_position);
        vector3_input("Velocity", "x", "y", "z", &physics.orig_velocity);
    }

    // Appearance options
    if (ImGui::CollapsingHeader("Appearance")) {
        // Configure the colour slider
        ImGuiColorEditFlags flags = 0;
        flags |= ImGuiColorEditFlags_NoAlpha;
        flags |= ImGuiColorEditFlags_NoSidePreview;
        flags |= ImGuiColorEditFlags_PickerHueWheel;
        flags |= ImGuiColorEditFlags_DisplayRGB;

        ImGui::ColorPicker4(
            "##Colour", 
            (float*) glm::value_ptr(instance.colour), 
            flags, 
            nullptr);
        ImGui::Checkbox("emits light", (bool*) &instance.emits_light);
    }

    physics.position = physics.orig_position;
    physics.velocity = physics.orig_velocity;
}

void SimulationFrontend::ui_state_switching()
{
    // State switching
    bool paused  = simulation.state == SimulationState::Paused;
    bool running = simulation.state == SimulationState::Running;
    bool waiting = simulation.state == SimulationState::Waiting;

    if (waiting) {
        if (ImGui::Button("Run simulation")) {
            simulation.state = SimulationState::Running;
            simulation.clear_tracers();
        }
    } else {
        if (running && ImGui::Button("Pause")) {
            simulation.state = SimulationState::Paused;

        } else if (paused && ImGui::Button("Resume")) {
            simulation.state = SimulationState::Running;
        }

        ImGui::SameLine();

        if (ImGui::Button("Reset")) {
            simulation.state = SimulationState::Waiting;
        }
    }
}

void SimulationFrontend::ui_body_selection()
{
    // Draw a selection menu containing all the 
    // bodies currently in the simulation
    auto select = [&](const char *text, auto&& selected_body, bool clear) {
        if (ImGui::Button(text)) {
            ImGui::OpenPopup(text);
        }

        // Name display
        ImGui::SameLine();
        std::string selected_body_name = simulation
            .get_info(selected_body)
            .name;
        ImGui::Text(selected_body_name.c_str());

        // Draw a selectable box for each body as well as one for no body
        if (ImGui::BeginPopup(text)) {
            if (ImGui::Selectable("NONE")) {
                selected_body = Simulation::NO_BODY;
            }

            for (int i = 0; i < simulation.num_bodies; ++i) {
                auto name = simulation.body_info[i].name; 
                if (i == selected_body) {
                    name = "<" + name + ">";
                }
                name += "##" + std::to_string(i); // Make unique
                if (ImGui::Selectable(name.c_str())) {
                    selected_body = i;
                    // Switching relative body means the
                    // tracers should be cleared
                    if (clear) {
                        simulation.clear_tracers();
                    }
                }
            }
            ImGui::EndPopup();
        }
    };

    select(
        "Select body to track", 
        tracked_body, 
        false);
    select(
        "Select body to draw relative to", 
        simulation.draw_tracers_relative_to,
        true);
}

void SimulationFrontend::ui_state_specifics()
{
    // State specific options
    if (simulation.state == SimulationState::Waiting) {
        // Add new body
        if (ImGui::Button("Add new body")) {
            ImGui::OpenPopup("body_add");
        }
        if (ImGui::BeginPopup("body_add")) {
            // Editing for a new body to be added
            ui_body_editing(prototype_info, prototype_physics, prototype_instance);

            if (ImGui::Button("Add") && prototype_info.name.size() > 0) {
                simulation.body_info.push_back(prototype_info);
                simulation.body_physics.push_back(prototype_physics);
                simulation.body_instance.push_back(prototype_instance);

                tracked_body = simulation.num_bodies;
                simulation.num_bodies++;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Checkbox("Show trajectories", &render_tracers);

        // Current Body options
        if (tracked_body != Simulation::NO_BODY) {
            ImGui::Text("Selected body:");
            auto& info = simulation.body_info[tracked_body];
            auto& phys = simulation.body_physics[tracked_body];
            auto& inst = simulation.body_instance[tracked_body];
            ui_body_editing(info, phys, inst);

            if (ImGui::Button("Draw relative to body")) {
                simulation.draw_tracers_relative_to = tracked_body;
            }

            if (ImGui::Button("Delete body")) {
                simulation.delete_body(tracked_body);
                tracked_body = Simulation::NO_BODY;
            }
        }
    } else {
        ImGui::Checkbox("Show trails", &render_tracers);
    }
}

void SimulationFrontend::show_ui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Options");

    ui_state_switching();
    ui_body_selection();
    ui_state_specifics();
    
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

