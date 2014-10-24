#ifndef HPP_CUSTOM_SHADERS
#define HPP_CUSTOM_SHADERS

#include <xd/graphics/shader_program.hpp>

class custom_shader : public xd::shader_program {
public:
    custom_shader(const std::string& vertex_src, const std::string& fragment_src);
};

#endif
