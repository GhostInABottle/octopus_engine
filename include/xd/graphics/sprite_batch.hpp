#ifndef H_XD_GRAPHICS_SPRITE_BATCH
#define H_XD_GRAPHICS_SPRITE_BATCH

#include <vector>
#include "detail/sprite_batch.hpp"
#include "vertex_batch.hpp"
#include "../ref_counted.hpp"
#include "transform_geometry.hpp"
#include "texture.hpp"
#include "types.hpp"
#include "../glm.hpp"
#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>

#pragma warning(disable: 4275)

namespace xd
{
    class shader_program;
    // sprite batch
    class XD_API sprite_batch : public xd::ref_counted, public boost::noncopyable
    {
    public:
        typedef boost::intrusive_ptr<sprite_batch> ptr;

        sprite_batch();
        virtual ~sprite_batch();

        void clear();

        typedef std::vector<xd::vertex_batch<detail::sprite_vertex_traits>::ptr> batch_list;
        batch_list create_batches();

        void draw(const xd::mat4& mvp_matrix, const batch_list& batches, int ticks = 0);
        void draw(const mat4& matrix, int ticks = 0);
        void draw(xd::shader_program& shader, const xd::mat4& mvp_matrix, const batch_list& batches, int ticks);
        void draw(xd::shader_program& shader, const mat4& matrix, int ticks);
        void draw_outlined(const xd::mat4& mvp_matrix, const batch_list& batches, int ticks = 0);
        void draw_outlined(const mat4& matrix, int ticks = 0);

        void set_scale(float scale) { m_scale = scale; }
        float get_scale() const { return m_scale; }
        void set_outline_color(vec4 outline_color) { m_outline_color = outline_color; }
        vec4 get_outline_color() const { return m_outline_color; }

        void set_shader(shader_program* shader);

        void add(const texture::ptr texture, float x, float y,
            const vec4& color = vec4(1), const vec2& origin = vec2(0, 0));
        void add(const texture::ptr texture, float x, float y, float rotation, float scale,
            const vec4& color = vec4(1), const vec2& origin = vec2(0, 0));
        void add(const texture::ptr texture, float x, float y, float rotation, const vec2& scale,
            const vec4& color = vec4(1), const vec2& origin = vec2(0, 0));

        void add(const texture::ptr texture, const rect& src, float x, float y,
            const vec4& color = vec4(1), const vec2& origin = vec2(0, 0));
        void add(const texture::ptr texture, const rect& src, float x, float y, float rotation, float scale,
            const vec4& color = vec4(1), const vec2& origin = vec2(0, 0));
        void add(const texture::ptr texture, const rect& src, float x, float y,
            float rotation, const vec2& scale, const vec4& color = vec4(1),
            const vec2& origin = vec2(0, 0));

    private:
        detail::sprite_batch_data *m_data;
        float m_scale;
        vec4 m_outline_color;
    };
}

#endif
