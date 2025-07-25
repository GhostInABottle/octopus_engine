#include "custom_shaders.hpp"
#include "xd/graphics/vertex_traits.hpp"
#include <string>

Custom_Shader::Custom_Shader(const std::string& vertex_src, const std::string& fragment_src) {
    attach(GL_VERTEX_SHADER, vertex_src);
    attach(GL_FRAGMENT_SHADER, fragment_src);
    bind_attribute("vVertex", xd::VERTEX_POSITION);
    bind_attribute("vTexCoords", xd::VERTEX_TEXTURE);
    link();
}
