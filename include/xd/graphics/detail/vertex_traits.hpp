#ifndef H_XD_GRAPHICS_DETAIL_VERTEX_TRAITS
#define H_XD_GRAPHICS_DETAIL_VERTEX_TRAITS

#include "../../vendor/glew/glew.h"
#include "../../glm.hpp"
#include <type_traits>

namespace xd
{
    namespace detail
    {
        struct vertex_attribute
        {
            GLint size;
            GLenum type;
            const GLvoid *offset;
            GLboolean normalized;
            GLsizei stride;
        };

        template <typename T>
        struct vertex_attribute_type
        {
            enum { value = vertex_attribute_type<typename T::value_type>::value };
        };

        template <>
        struct vertex_attribute_type<GLbyte>
        {
            enum { value = GL_BYTE };
        };

        template <>
        struct vertex_attribute_type<GLubyte>
        {
            enum { value = GL_UNSIGNED_BYTE };
        };

        template <>
        struct vertex_attribute_type<GLshort>
        {
            enum { value = GL_SHORT };
        };

        template <>
        struct vertex_attribute_type<GLushort>
        {
            enum { value = GL_UNSIGNED_SHORT };
        };

        template <>
        struct vertex_attribute_type<GLint>
        {
            enum { value = GL_INT };
        };

        template <>
        struct vertex_attribute_type<GLuint>
        {
            enum { value = GL_UNSIGNED_INT };
        };

        template <>
        struct vertex_attribute_type<GLdouble>
        {
            enum { value = GL_DOUBLE };
        };

        template <>
        struct vertex_attribute_type<GLfloat>
        {
            enum { value = GL_FLOAT };
        };

        template <typename T, typename Enable = void>
        struct vertex_attribute_size;

        template <typename T>
        struct vertex_attribute_size<T, typename std::enable_if<std::is_arithmetic<T>::value>::type>
        {
            enum { value = 1 };
        };

        template <typename T>
        struct vertex_attribute_size<glm::tvec1<T>>
        {
            enum { value = 1 };
        };

        template <typename T>
        struct vertex_attribute_size<glm::tvec2<T>>
        {
            enum { value = 2 };
        };

        template <typename T>
        struct vertex_attribute_size<glm::tvec3<T>>
        {
            enum { value = 3 };
        };

        template <typename T>
        struct vertex_attribute_size<glm::tvec4<T>>
        {
            enum { value = 4 };
        };
    }
}

#endif
