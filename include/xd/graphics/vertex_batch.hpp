#ifndef H_XD_GRAPHICS_VERTEX_BATCH
#define H_XD_GRAPHICS_VERTEX_BATCH

#include "vertex_traits.hpp"
#include <boost/noncopyable.hpp>

namespace xd
{
    template <typename Traits>
    class vertex_batch : public boost::noncopyable
    {
    public:

        vertex_batch(GLenum draw_mode = GL_TRIANGLES, const Traits& traits = Traits())
            : m_draw_mode(draw_mode)
            , m_traits(traits)
            , m_count(0)
        {
            init();
        }

        vertex_batch(const void *data, size_t count, GLenum draw_mode = GL_TRIANGLES, const Traits& traits = Traits())
            : m_draw_mode(draw_mode)
            , m_traits(traits)
            , m_count(0)
        {
            init();
            load(data, count);
        }

        ~vertex_batch()
        {
            glDeleteBuffers(1, &m_vbo);
        }

        Traits get_traits()
        {
            return m_traits;
        }

        GLenum get_draw_mode()
        {
            return m_draw_mode;
        }

        void set_draw_mode(GLenum draw_mode)
        {
            m_draw_mode = draw_mode;
        }

        void load(const void *data, int count)
        {
            // bind the buffer
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

            // allocate the buffer and load the data
            size_t size = count * m_traits.vertex_size;
            glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
            m_count = count;

            // unbind the buffer
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        void render() const
        {
            render(0, m_count);
        }

        void render(int begin, int count) const
        {
            // bind the buffer
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

            // enable used vertex attribs
            for (auto i = m_traits.m_attribs.begin(); i != m_traits.m_attribs.end(); ++i) {
                glVertexAttribPointer(i->first, i->second.size, i->second.type, i->second.normalized, i->second.stride, i->second.offset);
                glEnableVertexAttribArray(i->first);
            }

            // draw the batch
            glDrawArrays(m_draw_mode, begin, count);

            // disable used vertex attribs
            for (auto i = m_traits.m_attribs.begin(); i != m_traits.m_attribs.end(); ++i) {
                glDisableVertexAttribArray(i->first);
            }

            // unbind the buffer
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

    private:
        Traits m_traits;
        GLenum m_draw_mode;
        GLuint m_vbo;
        int m_count;

        void init()
        {
            // generate a VBO
            glGenBuffers(1, &m_vbo);
        }
    };
}

#endif
