#include <print>

#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Window.hpp>
#include <glad/glad.h>

#include "Application.h"

#include "GUI.h"
#include "Graphics/OpenGL/GLUtils.h"
#include "Util/Keyboard.h"
#include "Util/Profiler.h"
#include "Util/TimeStep.h"

namespace
{
    void handle_event(const sf::Event& event, sf::Window& window, bool& show_debug_info,
                      bool& close_requested);
} // namespace

int main()
{
    sf::ContextSettings context_settings;
    context_settings.depthBits = 24;
    context_settings.stencilBits = 8;
    context_settings.antiAliasingLevel = 4;
    context_settings.majorVersion = 4;
    context_settings.minorVersion = 6;
    context_settings.attributeFlags = sf::ContextSettings::Debug;

    sf::Window window(sf::VideoMode::getDesktopMode(), "Tessellation",
                      sf::Style::None, sf::State::Fullscreen, context_settings);

    window.setVerticalSyncEnabled(true);
    if (!window.setActive(true))
    {
        std::println(std::cerr, "Failed to activate the window.");
        return EXIT_FAILURE;
    }

    if (!gladLoadGL())
    {
        std::println(std::cerr, "Failed to initialise OpenGL - Is OpenGL linked correctly?");
        return EXIT_FAILURE;
    }
    glClearColor(50.0f / 255.0f, 205.0f / 255.0f, 250.0f / 255.0f, 0);
    glViewport(0, 0, window.getSize().x, window.getSize().y);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    gl::enable_debugging();


    TimeStep updater{50};
    Profiler profiler;
    bool show_debug_info = false;

    // ;
    Application app{window};
    if (!app.init())
    {
        return -1;
    }

    if (!GUI::init(&window))
    {
        std::println(std::cerr, "Failed to initialise Imgui.");
        return EXIT_FAILURE;
    }

    Keyboard keyboard;

    // -------------------
    // ==== Main Loop ====
    // -------------------
    sf::Clock clock;
    while (window.isOpen())
    {
        GUI::begin_frame();
        bool close_requested = false;
        while (auto event = window.pollEvent())
        {
            GUI::event(window, *event);
            keyboard.update(*event);
            app.on_event(*event);
            handle_event(*event, window, show_debug_info, close_requested);
        }
        auto dt = clock.restart();

        // Update
        {
            auto& update_profiler = profiler.begin_section("Update");
            app.on_update(keyboard, dt);
            update_profiler.end_section();
        }

        // Fixed-rate update
        {
            auto& fixed_update_profiler = profiler.begin_section("Fixed Update");
            updater.update([&](sf::Time dt) { app.on_fixed_update(dt); });
            fixed_update_profiler.end_section();
        }

        // Render
        {
            auto& render_profiler = profiler.begin_section("Render");
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            app.on_render(show_debug_info);
            render_profiler.end_section();
        }

        // Show profiler
        profiler.end_frame();
        if (show_debug_info)
        {
            profiler.gui();
        }

        // --------------------------
        // ==== End Frame ====
        // --------------------------
        GUI::render();
        window.display();
        if (close_requested)
        {
            window.close();
        }
    }

    // --------------------------
    // ==== Graceful Cleanup ====
    // --------------------------
    GUI::shutdown();
}

namespace
{
    void handle_event(const sf::Event& event, sf::Window& window, bool& show_debug_info,
                      bool& close_requested)
    {
        if (event.is<sf::Event::Closed>())
        {
            close_requested = true;
        }
        else if (auto* key = event.getIf<sf::Event::KeyPressed>())
        {

            switch (key->code)
            {
                case sf::Keyboard::Key::Escape:
                    close_requested = true;
                    break;

                case sf::Keyboard::Key::F1:
                    show_debug_info = !show_debug_info;
                    break;

                default:
                    break;
            }
        }
    }
} // namespace
