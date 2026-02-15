#include "PickerTexture.h"

PickerTexture::PickerTexture(GLuint width, GLuint height)
    : fbo_{width, height}
{
}

bool PickerTexture::create()
{
    if (!fbo_.attach_colour(gl::InternalTextureFormat::RGBA32F).attach_depth_buffer().is_complete())
    {
        return false;
    }
    glNamedFramebufferDrawBuffer(fbo_.id, GL_COLOR_ATTACHMENT0);
}

void PickerTexture::bind_read() const
{
    fbo_.bind(gl::FramebufferTarget::ReadFramebuffer, false);
}

void PickerTexture::bind_write()
{
    fbo_.bind(gl::FramebufferTarget::DrawFramebuffer, false);
}

PickerPixel PickerTexture::read_pixel(glm::ivec2 position)
{
    bind_read();
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    PickerPixel pixel;
    glReadPixels(position.x, position.y, 1, 1, GL_RGB, GL_FLOAT, &pixel);

    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    return pixel;
}
