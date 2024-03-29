#ifndef H_XD_GRAPHICS_SPRITE_BATCH
#define H_XD_GRAPHICS_SPRITE_BATCH

#include "detail/sprite_batch.hpp"
#include "vertex_batch.hpp"
#include "types.hpp"
#include "../glm.hpp"
#include "shader_uniforms.hpp"
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
        bool empty() const;

        typedef std::vector<std::shared_ptr<xd::vertex_batch<detail::sprite_vertex_traits>>> batch_list;
        batch_list create_batches();

        void draw(const shader_uniforms& uniforms, const batch_list& batches);
        void draw(const shader_uniforms& uniforms);
        void draw(xd::shader_program& shader, const shader_uniforms& uniforms, const batch_list& batches);
        void draw(xd::shader_program& shader, const shader_uniforms& uniforms);
        void draw_outlined(const shader_uniforms& uniforms, const batch_list& batches);
        void draw_outlined(const shader_uniforms& uniforms);

        void set_scale(float scale) noexcept { m_scale = scale; }
        float get_scale() const noexcept { return m_scale; }
        void set_outline_color(vec4 outline_color) { m_outline_color = outline_color; }
        vec4 get_outline_color() const { return m_outline_color; }

        void set_shader(std::unique_ptr<shader_program> shader);
        void reset_shader();

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
