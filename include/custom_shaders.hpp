#ifndef HPP_CUSTOM_SHADERS
#define HPP_CUSTOM_SHADERS

#include "xd/graphics/shader_program.hpp"

class Custom_Shader : public xd::shader_program {
public:
    Custom_Shader(const std::string& vertex_src, const std::string& fragment_src);
};

#endif
