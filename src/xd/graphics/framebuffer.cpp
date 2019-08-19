#include <stdexcept>
#include "../../../include/xd/graphics/framebuffer.hpp"
#include "../../../include/xd/graphics/texture.hpp"


xd::framebuffer::framebuffer()
{
    if (extension_supported())
        glGenFramebuffersEXT(1, &m_buffer_id);
}

xd::framebuffer::~framebuffer()
{
    if (extension_supported())
        glDeleteFramebuffersEXT(1, &m_buffer_id);
}

void xd::framebuffer::bind() const
{
    if (extension_supported())
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_buffer_id);
}

void xd::framebuffer::unbind() const
{
    if (extension_supported())
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void xd::framebuffer::attach_color_texture(const xd::texture& texture, int slot) const
{
    if (!extension_supported())
        return;

    bind();

    auto attachment = static_cast<GLenum>(GL_COLOR_ATTACHMENT0_EXT + slot);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment,
        GL_TEXTURE_2D, texture.texture_id(), 0);

    GLenum error = glGetError();
    if (error == GL_INVALID_OPERATION) {
        throw std::exception("Could not attach texture to framebuffer object");
    }

    GLenum draw_buffers[1] = { attachment };
    glDrawBuffers(1, draw_buffers);
}

void xd::framebuffer::attach_depth_buffer(unsigned int id) const
{
    if (!extension_supported())
        return;

    bind();

    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
        GL_RENDERBUFFER_EXT, id);

    GLenum error = glGetError();
    if (error == GL_INVALID_OPERATION) {
        throw std::exception("Could not attach depth buffer to framebuffer object");
    }
}

std::tuple<bool, std::string> xd::framebuffer::check_complete() const
{
    auto result = std::tuple(true, std::string(""));
    if (!extension_supported())
        return result;
    GLenum status = glGetError();
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT && status != GL_NO_ERROR) {
        std::get<0>(result) = false;
        std::string error = "Unknown framebuffer error";
        switch (status) {
        case GL_INVALID_OPERATION:
            error = "Framebuffer operation is invalid";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            error = "Framebuffer attachment is incomplete";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            error = "A framebuffer attachment is missing";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            error = "Framebuffer format is not supported";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            error = "Framebuffer attachments have different formats";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            error = "Framebuffer attachments have different dimensions";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            error = "Framebuffer read buffer is missing";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            error = "Framebuffer draw buffer is missing";
            break;
        }
        std::get<1>(result) = error;
    }
    return result;
}

