#include "FileSelector.h"

#include <filesystem>
#include <iostream>

#include <imgui.h>

FileSelector::FileSelector()
{
    refresh();
}

void FileSelector::refresh()
{
    // When showing, the list must reset in case files have been saved, deleted, renamed etc
    // since it was last opened
    files_.clear();
    for (const auto& entry : std::filesystem::directory_iterator("./assets/heightmaps/"))
    {
        auto name = entry.path().filename().string();
        if (entry.is_regular_file())
        {
            files_.push_back(name);
        }
    }
}

std::optional<std::string> FileSelector::display_gui()
{
    std::optional<std::string> selection = std::nullopt;

    if (ImGui::Begin("Load Map"))
    {
        if (ImGui::Button("Refresh"))
        {
            refresh();
        }

        ImGui::Text("Select a map to load:");
        ImGui::Separator();

        for (const auto& file : files_)
        {

            if (ImGui::Button(file.c_str()))
            {
                selection = file;
            }
        }
    }
    ImGui::End();
    return selection;
}