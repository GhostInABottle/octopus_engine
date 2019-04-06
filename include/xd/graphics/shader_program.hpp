#ifndef H_XD_GRAPHICS_SHADER_PROGRAM
#define H_XD_GRAPHICS_SHADER_PROGRAM

#include "../config.hpp"
#include "../ref_counted.hpp"
#include "../vendor/glew/glew.h"
#include "../glm.hpp"
#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <string>
#include <unordered_map>

#pragma warning(disable: 4275)

namespace xd
{
    class XD_API shader_program : public xd::ref_counted, public boost::noncopyable
    {
    public:
        typedef boost::intrusive_ptr<shader_program> ptr;

        // constructors & destructors
        shader_program();
        virtual ~shader_program();

        // attach shader and link
        void attach(GLuint type, const std::string& src);
        void link();

        // use it
        virtual void use();
        virtual void setup();

        // bind attrib
        virtual void bind_attribute(const std::string& name, GLuint attr);

        // bind uniforms
        virtual void bind_uniform(const std::string& name, int val);
        virtual void bind_uniform(const std::string& name, float val);
        virtual void bind_uniform(const std::string& name, const glm::vec2& val);
        virtual void bind_uniform(const std::string& name, const glm::vec3& val);
        virtual void bind_uniform(const std::string& name, const glm::vec4& val);
        virtual void bind_uniform(const std::string& name, const glm::mat2& val);
        virtual void bind_uniform(const std::string& name, const glm::mat3& val);
        virtual void bind_uniform(const std::string& name, const glm::mat4& val);

        // get uniform location
        GLint get_uniform_location(const std::string& name);
    protected:
        // shader program and attrib list
        GLuint m_program;
        std::unordered_map<std::string, GLint> m_uniform_map;
    };
}

#endif
