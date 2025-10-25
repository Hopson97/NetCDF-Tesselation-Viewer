#pragma once

#include "OpenGL/Framebuffer.h"

struct PickerPixel
{
    float object_id = -1;
    float draw_id = -1;
    float primative_id = -1;
};

class PickerTexture
{
  public:
    PickerTexture(GLuint width, GLuint height);

    bool create();

    void bind_read() const;
    void bind_write();

    PickerPixel read_pixel(glm::ivec2 position);

  private:
    gl::Framebuffer fbo_;
};