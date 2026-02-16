#pragma once

#include <SFML/System/Clock.hpp>
#include <SFML/Window/Window.hpp>

#include "FileSelector.h"
#include "GUI.h"
#include "Graphics/Camera.h"
#include "Graphics/CameraController.h"
#include "Graphics/Mesh.h"
#include "Graphics/Model.h"
#include "Graphics/OpenGL/Shader.h"
#include "Graphics/OpenGL/Texture.h"
#include "Graphics/OpenGL/VertexArrayObject.h"
#include "HeightMapLoaders.h"
#include "Settings.h"
#include "Util/Maths.h"

class Keyboard;

struct TessellationSettings
{
    float height_scale = 2.0f;
    float min_distance = 250;
    float max_distance = 1000;
    int min_tess_level = 4;
    int max_tess_level = 256;
};

class Application
{
  public:
    Application(sf::Window& window);
    [[nodiscard]] bool init();

    void on_event(const sf::Event& event);
    void on_update(const Keyboard& keyboard, sf::Time dt);
    void on_fixed_update(sf::Time dt);
    void on_render(bool show_debug_info);

  private:
    void render_scene(gl::Shader& shader);

    void load_map(const std::string& name);

    Camera perspective_camera_;
    Camera ortho_camera_;
    Camera* p_active_camera_ = nullptr;
    CameraKeybinds camera_keybinds_;
    CameraControllerOptions camera_options_;

    Transform object_transform_;

    gl::Texture2D grass_material_;
    gl::Texture2D crate_material_;

    gl::BufferObject matrices_ssbo_;
    gl::Shader scene_shader_;
    gl::Shader terrain_shader_;

    Mesh3D terrain_mesh_;
    Mesh3D box_mesh_ = generate_cube_mesh({1.0f, 1.0f, 1.0f});

    TerrainHeightMaps terrain_heightmaps_;

    sf::Clock game_time_;

    Settings settings_;

    sf::Window& window_;

    Model model_;

    TessellationSettings tess_settings_;

    FileSelector file_selector_;
};