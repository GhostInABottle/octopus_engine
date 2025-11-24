#ifndef H_XD_GRAPHICS_SPRITE_BATCH
#define H_XD_GRAPHICS_SPRITE_BATCH

#include "detail/sprite_batch.hpp"
#include "vertex_batch.hpp"
#include "types.hpp"
#include "../glm.hpp"
#include <vector>
#include <memory>
#include <string>
#include <variant>

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

        typedef std::vector<std::shared_ptr<vertex_batch<detail::sprite_vertex_traits>>> batch_list;
        batch_list create_batches();

        void draw(mat4 mvp_matrix, const batch_list& batches);
        void draw(mat4 mvp_matrix);
        void draw(xd::shader_program& shader, mat4 mvp_matrix, const batch_list& batches);
        void draw(xd::shader_program& shader, mat4 mvp_matrix);
        void draw_outlined(mat4 mvp_matrix, const batch_list& batches);
        void draw_outlined(mat4 mvp_matrix);

        void set_scale(float scale) noexcept { m_scale = scale; }
        float get_scale() const noexcept { return m_scale; }
        void set_outline_color(vec4 outline_color) { m_outline_color = outline_color; }
        vec4 get_outline_color() const { return m_outline_color; }

        void set_shader(std::unique_ptr<shader_program> shader);
        void reset_shader();

        typedef std::variant<int, float, xd::vec2, xd::vec3, xd::vec4,
            xd::mat2, xd::mat3, xd::mat4> uniform_types;
        void set_uniform(const std::string& name, uniform_types val);

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
        std::unordered_map<std::string, uniform_types> m_uniforms;
        float m_scale;
        vec4 m_outline_color;
        void bind_uniform(const std::string& name, const uniform_types& variant);
    };
}

#endif
