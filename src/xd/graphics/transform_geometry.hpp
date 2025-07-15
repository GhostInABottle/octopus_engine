#ifndef H_XD_GRAPHICS_TRANSFORM_GEOMETRY
#define H_XD_GRAPHICS_TRANSFORM_GEOMETRY

#include "matrix_stack.hpp"

namespace xd
{
    class transform_geometry
    {
    public:
        matrix_stack& projection() noexcept
        {
            return m_projection;
        }

        const matrix_stack& projection() const noexcept
        {
            return m_projection;
        }

        matrix_stack& model_view() noexcept
        {
            return m_model_view;
        }

        const matrix_stack& model_view() const noexcept
        {
            return m_model_view;
        }

        glm::mat4 mvp() const
        {
            return m_projection.get() * m_model_view.get();
        }

    private:
        matrix_stack m_projection;
        matrix_stack m_model_view;
    };
}

#endif
