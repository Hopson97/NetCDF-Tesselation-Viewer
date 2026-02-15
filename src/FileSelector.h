#pragma once

#include <vector>
#include <string>
#include <optional>

class FileSelector
{
  public:
    FileSelector();
    void refresh();

    bool is_showing() const;

    std::optional<std::string> display_gui();

  private:
    std::vector<std::string> files_;
};
