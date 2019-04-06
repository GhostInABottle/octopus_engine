#ifndef H_XD_GRAPHICS_FONT
#define H_XD_GRAPHICS_FONT

#include "detail/font.hpp"
#include "font_style.hpp"
#include "../vendor/glew/glew.h"
#include "../glm.hpp"
#include "../ref_counted.hpp"
#include "types.hpp"
#include "vertex_batch.hpp"
#include "shader_program.hpp"
#include "transform_geometry.hpp"
#include "../vendor/utf8.h"
#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <memory>
#include <unordered_map>

#ifndef XD_STATIC
// disable warning about boost::noncopyable not being dll-exportable
// as well as the private members that can't be accessed by client
#pragma warning(disable: 4275 4251 4231)

// export instantiantions of used stdlib classes
//XD_API_TEMPLATE template class XD_API std::allocator<char>;
//XD_API_TEMPLATE template class XD_API std::basic_string<char, std::char_traits<char>, std::allocator<char> >;
#endif

namespace xd
{
    // font class
    class XD_API font : public xd::ref_counted, public boost::noncopyable
    {
    public:
        typedef boost::intrusive_ptr<font> ptr;

        font(const std::string& filename);
        virtual ~font();

        void link_font(const std::string& type, const std::string& filename);
        void link_font(const std::string& type, font::ptr font);
        void unlink_font(const std::string& type);

        void render(const std::string& text, const font_style& style,
            shader_program::ptr shader, const glm::mat4& mvp,
            glm::vec2 *pos = 0, bool actual_rendering = true);

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
            std::size_t operator () (const std::pair<int, int> &p) const {
                std::size_t seed = 0;
                seed ^= p.first + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                seed ^= p.second + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                return  seed;
            }
        };
        typedef std::unordered_map<std::pair<int, int>, std::unique_ptr<detail::font::glyph>, int_pair_hash> glyph_map_t;
        typedef std::unordered_map<std::string, font::ptr> font_map_t;
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
