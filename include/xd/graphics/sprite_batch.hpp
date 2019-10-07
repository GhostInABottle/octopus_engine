#ifndef H_XD_GRAPHICS_SPRITE_BATCH
#define H_XD_GRAPHICS_SPRITE_BATCH

#include "detail/sprite_batch.hpp"
#include "vertex_batch.hpp"
#include "transform_geometry.hpp"
#include "types.hpp"
#include "../glm.hpp"
#include <vector>
#include <memory>

namespace xd
{
    class shader_program;
    class texture;
    // sprite batch
    class sprite_batch
    {
    public:
        sprite_batch(const sprite_batch&) = delete;
        sprite_batch& operator=(const sprite_batch&) = delete;
        sprite_batch();
        virtual ~sprite_batch();

        void clear();

        typedef std::vector<std::shared_ptr<xd::vertex_batch<detail::sprite_vertex_traits>>> batch_list;
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

        void set_shader(std::unique_ptr<shader_program> shader);

        void add(const std::shared_ptr<texture>& texture, float x, float y,
            const vec4& color = vec4(1), const vec2& origin = vec2(0, 0));
        void add(const std::shared_ptr<texture>& texture, float x, float y, float rotation, float scale,
            const vec4& color = vec4(1), const vec2& origin = vec2(0, 0));
        void add(const std::shared_ptr<texture>& texture, float x, float y, float rotation, const vec2& scale,
            const vec4& color = vec4(1), const vec2& origin = vec2(0, 0));

        void add(const std::shared_ptr<texture>& texture, const rect& src, float x, float y,
            const vec4& color = vec4(1), const vec2& origin = vec2(0, 0));
        void add(const std::shared_ptr<texture>& texture, const rect& src, float x, float y, float rotation, float scale,
            const vec4& color = vec4(1), const vec2& origin = vec2(0, 0));
        void add(const std::shared_ptr<texture>& texture, const rect& src, float x, float y,
            float rotation, const vec2& scale, const vec4& color = vec4(1),
            const vec2& origin = vec2(0, 0));

    private:
        std::unique_ptr<detail::sprite_batch_data> m_data;
        std::unique_ptr<xd::vertex_batch<detail::sprite_vertex_traits>> m_batch;
        float m_scale;
        vec4 m_outline_color;
    };
}

#endif
