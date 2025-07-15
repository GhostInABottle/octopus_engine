#ifndef H_XD_GRAPHICS_SHADERS
#define H_XD_GRAPHICS_SHADERS

#include "shader_program.hpp"
#include "texture.hpp"

namespace xd
{
    class flat_shader : public shader_program
    {
    public:
        flat_shader();
        virtual void setup(const glm::mat4& mvp, const glm::vec4& color);
    };

    class text_shader : public shader_program
    {
    public:
        text_shader();
    };

    class sprite_shader : public shader_program
    {
    public:
        sprite_shader();
    };

    class sprite_outline_shader : public shader_program
    {
    public:
        sprite_outline_shader();
    };

    class fullscreen_shader : public shader_program
    {
    public:
        fullscreen_shader();
    };
}

#endif
