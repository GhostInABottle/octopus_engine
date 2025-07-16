#ifndef H_XD_GRAPHICS_FONT
#define H_XD_GRAPHICS_FONT

#include "../glm.hpp"
#include "../vendor/utf8.h"
#include "font_style.hpp"
#include "shader_program.hpp"
#include <iosfwd>
#include <memory>
#include <optional>
#include <unordered_map>

namespace xd
{
    namespace detail::font {
        struct glyph;
        struct face;
    }

    // font class
    class font
    {
    public:
        // Ownership of the stream will be transferred to the font
        font(const std::string& filename, std::unique_ptr<std::istream> stream);
        virtual ~font();
        font(const font&) = delete;
        font& operator=(const font&) = delete;

        void link_font(const std::string& type, const std::string& filename, std::unique_ptr<std::istream> stream);
        void link_font(const std::string& type, std::shared_ptr<font> font);
        void unlink_font(const std::string& type);

        glm::vec2 render(const std::string& text, const font_style& style,
            shader_program* shader, const glm::mat4& mvp,
            std::optional<glm::vec2> pos = std::nullopt, bool actual_rendering = true);

        float get_width(const std::string& text, const font_style& style);

        const std::string& get_mvp_uniform();
        const std::string& get_pos_uniform();
        const std::string& get_color_uniform();
        const std::string& get_texture_uniform();

        void set_mvp_uniform(const std::string&);
        void set_pos_uniform(const std::string&);
        void set_color_uniform(const std::string&);
        void set_texture_uniform(const std::string&);

        std::string filename() const { return m_filename; }
    private:
        struct int_pair_hash {
            std::size_t operator () (const std::pair<int, int> &p) const noexcept {
                std::size_t seed = 0;
                seed ^= p.first + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                seed ^= p.second + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                return  seed;
            }
        };
        typedef std::unordered_map<std::pair<int, int>, std::unique_ptr<detail::font::glyph>, int_pair_hash> glyph_map_t;
        typedef std::unordered_map<std::string, std::shared_ptr<font>> font_map_t;
        void load_size(int size, int load_flags);
        const detail::font::glyph& load_glyph(utf8::uint32_t char_index, int size, int load_flags);

        std::unique_ptr<detail::font::face> m_face;
        std::string m_filename;
        glyph_map_t m_glyph_map;
        font_map_t m_linked_fonts;
        std::string m_mvp_uniform;
        std::string m_position_uniform;
        std::string m_color_uniform;
        std::string m_texture_uniform;
    };
}

#endif
