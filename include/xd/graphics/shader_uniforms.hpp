#ifndef H_XD_GRAPHICS_SHADER_UNIFORMS
#define H_XD_GRAPHICS_SHADER_UNIFORMS

#include "../glm.hpp"
#include <optional>

namespace xd
{
    struct shader_uniforms {
        mat4 mvp_matrix;
        std::optional<int> ticks;
        std::optional<float> brightness;
        std::optional<float> contrast;
        shader_uniforms(mat4 mvp_matrix,
            std::optional<int> ticks = std::nullopt,
            std::optional<float> brightness = std::nullopt,
            std::optional<float> contrast = std::nullopt)
            : mvp_matrix(mvp_matrix), ticks(ticks), brightness(brightness), contrast(contrast) {}
    };

}

#endif