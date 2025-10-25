#include "Application.h"

#include <SFML/Window/Event.hpp>
#include <imgui.h>

#include "Graphics/CameraController.h"
#include "Graphics/Lights.h"
#include "Graphics/OpenGL/GLUtils.h"
#include "Util/Maths.h"
#include "Util/Util.h"

Application::Application(sf::Window& window)
    : perspective_camera_(CameraConfig{
          .type = CameraType::Perspective,
          .viewport_size = {window.getSize().x, window.getSize().y},
          .near = 0.1f,
          .far = 2000.0f,
          .fov = 90.0f,
      })
    , ortho_camera_(CameraConfig{
          .type = CameraType::OrthographicWorld,
          .viewport_size = {window.getSize().x, window.getSize().y},
          .near = 0.5f,
          .far = 1000.0f,
      })
    , window_(window)
{
    p_active_camera_ = &perspective_camera_;
}

bool Application::init()
{
    window_.setMouseCursorVisible(false);

    // -----------------------
    // ==== Load textures ====
    // -----------------------
    if (!grass_material_.load_from_file("assets/textures/grass.png", 8, false, false) ||
        !crate_material_.load_from_file("assets/textures/crate.png", 8, false, false) ||
        !terrain_heightmap_.load_from_file(
            HEIGHTMAP, 8, false, false,
            gl::TextureParameters{.min_filter = gl::TextureMinFilter::Nearest,
                                  .mag_filter = gl::TextureMagFilter::Nearest,
                                  .wrap_s = gl::TextureWrap::ClampToEdge,
                                  .wrap_t = gl::TextureWrap::ClampToEdge}))
    {
        return false;
    }

    // --------------------
    // ==== Load model ====
    // --------------------
    ModelLoader model_loader;
    auto model = model_loader.load_model("assets/models/box/box.obj");
    if (model)
    {
        model_ = std::move(*model);
    }

    // ---------------------------
    // ==== Buffer thr meshes ====
    // ---------------------------
    terrain_mesh_.buffer_no_ebo();

    box_mesh_.buffer();

    // ----------------------------
    // ==== Load scene shaders ====
    // ----------------------------
    if (!scene_shader_.load_stage("assets/shaders/Scene/SceneVertex.glsl",
                                  gl::ShaderType::Vertex) ||
        !scene_shader_.load_stage("assets/shaders/Scene/SceneFragment.glsl",
                                  gl::ShaderType::Fragment) ||
        !scene_shader_.link_shaders())
    {
        return false;
    }
    scene_shader_.set_uniform("diffuse", 0);

    if (!terrain_shader_.load_stage("assets/shaders/Terrain/TerrainVertex.glsl",
                                    gl::ShaderType::Vertex) ||
        !terrain_shader_.load_stage("assets/shaders/Terrain/TerrainTessControl.glsl",
                                    gl::ShaderType::TesselationControl) ||
        !terrain_shader_.load_stage("assets/shaders/Terrain/TerrainTessEval.glsl",
                                    gl::ShaderType::TesselationEvaluation) ||
        !terrain_shader_.load_stage("assets/shaders/Terrain/TerrainFragment.glsl",
                                    gl::ShaderType::Fragment) ||
        !terrain_shader_.link_shaders())
    {
        return false;
    }
    terrain_shader_.set_uniform("height_map", 0);

    // -------------------------
    // ==== Set up the SSBO ====
    // -------------------------
    matrices_ssbo_.create_as_ssbo<glm::mat4>(0, 2);

    // -----------------------------------
    // ==== Entity Transform Creation ====
    // -----------------------------------
    perspective_camera_.transform = {.position = {3.0f, 4.0f, 16.0f},
                                     .rotation = {4.0f, 300.0f, 0.0f}};
    ortho_camera_.transform = {.position = {6, 20, 5}, .rotation = {333.4f, 45.0f, 0.0f}};

    object_transform_ = {.position = {5.0f, 3.0f, 5.0f}};

    return true;
}

void Application::on_event(const sf::Event& event)
{
    if (event.is<sf::Event::KeyReleased>())
    {
        auto* key = event.getIf<sf::Event::KeyReleased>();

        switch (key->code)
        {
            case sf::Keyboard::Key::L:
                camera_options_.lock_rotation = !camera_options_.lock_rotation;
                window_.setMouseCursorVisible(camera_options_.lock_rotation);
                break;

            default:
                break;
        }
    }
}

void Application::on_update(const Keyboard& keyboard, sf::Time dt)
{
    free_camera_controller(keyboard, *p_active_camera_, dt, camera_keybinds_, window_,
                           camera_options_);
}

void Application::on_fixed_update(sf::Time dt)
{
    sf::Time game_time_now = game_time_.getElapsedTime();
    object_transform_.position.x +=
        glm::sin(game_time_now.asSeconds() * 0.55f) * dt.asSeconds() * 2.0f;
    object_transform_.position.z +=
        glm::cos(game_time_now.asSeconds() * 0.55f) * dt.asSeconds() * 2.0f;
}

void Application::on_render(bool show_debug_info)
{
    if (show_debug_info)
    {
        GUI::debug_window(perspective_camera_.transform.position,
                          perspective_camera_.transform.rotation, settings_);
        p_active_camera_->gui("Camera");
    }

    if (ImGui::Begin("Tessellation Settings"))
    {
        ImGui::SliderFloat("Height Scale", &tess_settings_.height_scale, 1.0f, 128.0f);
        ImGui::SliderFloat("Height Offset", &tess_settings_.height_offset, 1.0f, 128.0f);
        ImGui::SliderFloat("Min Distance", &tess_settings_.min_distance, 1.0f, 1000.0f);
        ImGui::SliderFloat("Max Distance", &tess_settings_.max_distance, 1.0f, 1000.0f);
        ImGui::SliderInt("Min Level", &tess_settings_.min_tess_level, 1, 128);
        ImGui::SliderInt("Max Level", &tess_settings_.max_tess_level, 1, 256);
    }
    ImGui::End();

    if (ImGui::Begin("Camera"))
    {
        if (ImGui::Button("Use Perspective"))
        {
            p_active_camera_ = &perspective_camera_;
        }
        if (ImGui::Button("Use Ortho"))
        {
            p_active_camera_ = &ortho_camera_;
        }
    }
    ImGui::End();

    // Update the shader buffers
    matrices_ssbo_.buffer_sub_data(0, p_active_camera_->get_projection_matrix());
    matrices_ssbo_.buffer_sub_data(sizeof(glm::mat4), p_active_camera_->get_view_matrix());

    scene_shader_.bind();
    render_scene(scene_shader_);

    terrain_shader_.bind();
    terrain_shader_.set_uniform("view_matrix", p_active_camera_->get_view_matrix());
    terrain_shader_.set_uniform("model_matrix", create_model_matrix({.position = {0, 0, 0}}));
    terrain_shader_.set_uniform("height_offset", tess_settings_.height_offset);
    terrain_shader_.set_uniform("height_scale", tess_settings_.height_scale);
    terrain_shader_.set_uniform("min_distance", tess_settings_.min_distance);
    terrain_shader_.set_uniform("max_distance", tess_settings_.max_distance);
    terrain_shader_.set_uniform("min_tess_level", tess_settings_.min_tess_level);
    terrain_shader_.set_uniform("max_tess_level", tess_settings_.max_tess_level);
    terrain_heightmap_.bind(0);
    terrain_mesh_.bind();

    glDrawArrays(GL_PATCHES, 0, 4 * 32 * 32);

    // Ensure GUI etc are rendered using fill
    gl::polygon_mode(gl::Face::FrontAndBack, gl::PolygonMode::Fill);
}

void Application::render_scene(gl::Shader& shader)
{
    // Set up the capabilities/ render states
    gl::enable(gl::Capability::DepthTest);
    // gl::enable(gl::Capability::CullFace);
    gl::cull_face(gl::Face::Back);
    gl::polygon_mode(gl::Face::FrontAndBack,
                     settings_.wireframe ? gl::PolygonMode::Line : gl::PolygonMode::Fill);

    // Draw terrain
    gl::Texture2D& texture = settings_.use_grass_texture ? grass_material_ : crate_material_;
    texture.bind(0);
    shader.set_uniform("model_matrix", create_model_matrix({.position = {-5, 0, -5}}));

    // Draw a box
    crate_material_.bind(0);
    box_mesh_.bind();
    shader.set_uniform("model_matrix", create_model_matrix({.position = {5, 0, 5}}));
    box_mesh_.draw_elements();

    // Set the floating box transform and render
    shader.set_uniform("model_matrix", create_model_matrix(object_transform_));
    box_mesh_.draw_elements();

    // Draw a loaded model
    shader.set_uniform("model_matrix", create_model_matrix({.position = {12, 1, 12}}));
    model_.draw(shader);
}
