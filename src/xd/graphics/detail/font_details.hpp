#ifndef H_XD_GRAPHICS_DETAIL_FONT
#define H_XD_GRAPHICS_DETAIL_FONT
#include "../vertex_traits.hpp"
#include "../vertex_batch.hpp"
#include "../../glm.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <memory>
#include <iosfwd>
#include <string>
#include <unordered_map>

namespace xd::detail:: font {
    class ft_lib {
    public:
        ft_lib();
        ~ft_lib();
        operator FT_Library() { return m_library; }
    private:

        FT_Library m_library;
    };

    struct vertex {
        glm::vec2 pos;
        glm::vec2 tex;
    };

    struct vertex_traits : public xd::vertex_traits<vertex> {
        vertex_traits() {
            // bind vertex attributes
            bind_vertex_attribute(VERTEX_POSITION, &vertex::pos);
            bind_vertex_attribute(VERTEX_TEXTURE, &vertex::tex);
        }
    };

    typedef vertex_batch<vertex_traits> vertex_batch_t;
    typedef std::shared_ptr<vertex_batch_t> vertex_batch_ptr_t;

    struct glyph {
        glyph() : glyph_index(0), texture_id(0) {}
        glyph(const glyph& other)
            : glyph_index(other.glyph_index)
            , texture_id(other.texture_id)
            , quad_ptr(other.quad_ptr)
            , advance(other.advance)
            , offset(other.offset) {}

        FT_UInt glyph_index;
        GLuint texture_id;
        vertex_batch_ptr_t quad_ptr;
        glm::vec2 advance, offset;
    };

    unsigned long file_read(FT_Stream rec, unsigned long offset, unsigned char* buffer, unsigned long count);
    void file_close(FT_Stream);

    struct face {
        face(std::shared_ptr<ft_lib> lib, const std::string& filename, std::unique_ptr<std::istream> stream);
        ~face();
        std::shared_ptr<ft_lib> library_ptr;
        FT_Face handle;
        hb_font_t* hb_font;
        hb_buffer_t* hb_buffer;
        std::unordered_map<int, FT_Size> sizes;
        std::unique_ptr<FT_StreamRec> stream_rec;
        std::unique_ptr<std::istream> istream;
    };

    hb_script_t ucdn_get_script(hb_codepoint_t codepoint);
}

#endif
