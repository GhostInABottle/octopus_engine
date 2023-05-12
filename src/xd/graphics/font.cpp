#include "../../../include/xd/graphics/font.hpp"
#include "../../../include/xd/graphics/exceptions.hpp"
#include "../../../include/xd/vendor/utf8.h"
#include "../../../include/log.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include "../../../include/xd/vendor/unicode_data.hpp"
#include <memory>
#include <unordered_map>
#include <memory>
#include <istream>
#include <mutex>

namespace xd { namespace detail { namespace font {

    class ft_lib {
    public:
        ft_lib() : m_library(nullptr) {
            int error = FT_Init_FreeType(&m_library);
            if (error)
                throw freetype_init_failed();
        }
        ~ft_lib() {
            FT_Done_FreeType(m_library);
        }
        operator FT_Library() { return m_library; }
    private:
        FT_Library m_library;
    };

    static ft_lib ft_library;

    struct vertex
    {
        glm::vec2 pos;
        glm::vec2 tex;
    };

    struct vertex_traits : public xd::vertex_traits<vertex>
    {
        vertex_traits()
        {
            // bind vertex attribs
            bind_vertex_attribute(VERTEX_POSITION, &vertex::pos);
            bind_vertex_attribute(VERTEX_TEXTURE, &vertex::tex);
        }
    };

    typedef vertex_batch<vertex_traits> vertex_batch_t;
    typedef std::shared_ptr<vertex_batch_t> vertex_batch_ptr_t;

    struct glyph
    {
        glyph() : glyph_index(0), texture_id(0) {}
        glyph(const glyph& other)
            : glyph_index(other.glyph_index)
            , texture_id(other.texture_id)
            , quad_ptr(other.quad_ptr)
            , advance(other.advance)
            , offset(other.offset)
        {}

        FT_UInt glyph_index;
        GLuint texture_id;
        vertex_batch_ptr_t quad_ptr;
        glm::vec2 advance, offset;
    };

    unsigned long file_read(FT_Stream rec, unsigned long offset, unsigned char* buffer, unsigned long count) {
        auto* istream = static_cast<std::istream*>(rec->descriptor.pointer);
        istream->clear();

        if (count > 0) {
            istream->read(reinterpret_cast<char*>(buffer), count);
            // Read operation
            if (!*istream || istream->eof()) return 0;

            return static_cast<unsigned long>(istream->gcount());
        }

        // Seek operation
        istream->seekg(offset, std::ios::beg);
        return *istream ? 0 : 1;
    }
    void file_close(FT_Stream) {}

    // FT Face ops need to be thread-safe
    // see https://freetype.org/freetype2/docs/reference/ft2-base_interface.html#ft_library
    static std::mutex face_mutex;

    struct face
    {
        face(const std::string& filename, std::unique_ptr<std::istream> stream)
                : stream_rec{ std::make_unique<FT_StreamRec>() },
                istream{ std::move(stream) } {
            if (!istream || !*istream) throw font_load_failed(filename);

            // calculate file size
            istream->seekg(0, std::ios::end);
            auto file_size = static_cast<unsigned long>(istream->tellg());
            istream->seekg(0, std::ios::beg);

            LOGGER_D << "Loading font " << filename << " with size " << file_size;

            // use custom file access functions
            stream_rec->base = nullptr;
            stream_rec->size = file_size;
            stream_rec->pos = 0;
            stream_rec->descriptor.pointer = static_cast<void*>(istream.get());
            stream_rec->read = file_read;
            stream_rec->close = file_close;

            FT_Open_Args args{};
            args.flags = FT_OPEN_STREAM;
            args.stream = stream_rec.get();

            std::lock_guard<std::mutex> lock{ face_mutex };

            // load the font
            auto error = FT_Open_Face(ft_library, &args, 0, &handle);
            if (error) throw font_load_failed(filename);

            hb_font = hb_ft_font_create(handle, 0);
            hb_buffer = hb_buffer_create();
            hb_buffer_set_content_type(hb_buffer, HB_BUFFER_CONTENT_TYPE_UNICODE);
        }
        ~face() {
            std::lock_guard<std::mutex> lock{ face_mutex };
            hb_buffer_destroy(hb_buffer);
            hb_font_destroy(hb_font);
            // free font sizes
            for (auto& i : sizes) {
                FT_Done_Size(i.second);
            }
            // free the font handle
            FT_Done_Face(handle);
        }
        FT_Face handle;
        hb_font_t* hb_font;
        hb_buffer_t* hb_buffer;
        std::unordered_map<int, FT_Size> sizes;
        std::unique_ptr<FT_StreamRec> stream_rec;
        std::unique_ptr<std::istream> istream;
    };

} } }

using namespace xd::detail::font;

xd::font::font(const std::string& font_filename, std::istream* stream)
        : m_filename(font_filename)
        , m_mvp_uniform("mvpMatrix")
        , m_position_uniform("vPosition")
        , m_color_uniform("vColor")
        , m_texture_uniform("colorMap")
        , m_face(std::make_unique<face>(font_filename,
            std::unique_ptr<std::istream>(stream))) {
}

xd::font::~font()
{
    // free all textures
    for (auto i = m_glyph_map.begin(); i != m_glyph_map.end(); ++i) {
        glDeleteTextures(1, &i->second->texture_id);
    }
}

void xd::font::link_font(const std::string& type, const std::string& filename, std::unique_ptr<std::istream> stream)
{
    auto linked_font = std::make_shared<font>(filename, stream.release());
    m_linked_fonts[type] = linked_font;
}

void xd::font::link_font(const std::string& type, std::shared_ptr<font> font)
{
    m_linked_fonts[type] = font;
}

void xd::font::unlink_font(const std::string& type)
{
    m_linked_fonts.erase(type);
}

void xd::font::load_size(int size, int load_flags)
{
    // create a new size
    FT_Size font_size;
    if (FT_New_Size(m_face->handle, &font_size) != 0)
        throw font_load_failed(m_filename);
    // free the size in catch block if something goes wrong
    try {
        if (FT_Activate_Size(font_size) != 0)
            throw font_load_failed(m_filename);
        // set the pixel size
        if (FT_Set_Pixel_Sizes(m_face->handle, 0, size) != 0)
            throw font_load_failed(m_filename);
        // pre-load 7-bit ASCII glyphs
        for (int i = 0; i < 128; i++) {
            load_glyph(i, size, load_flags);
        }
    } catch (...) {
        // free the size and re-throw
        FT_Done_Size(font_size);
        throw;
    }
    // insert the newly loaded size in the map
    m_face->sizes.emplace(size, font_size);
}

const xd::detail::font::glyph& xd::font::load_glyph(utf8::uint32_t char_index, int size, int load_flags)
{
    // check if glyph is already loaded
    auto key = std::make_pair(char_index, size);
    glyph_map_t::iterator i = m_glyph_map.find(key);
    if (i != m_glyph_map.end())
        return *i->second;

    int error = FT_Load_Glyph(m_face->handle, char_index, load_flags);
    if (error)
        throw glyph_load_failed(m_filename, char_index);

    error = FT_Render_Glyph(m_face->handle->glyph, FT_RENDER_MODE_NORMAL);
    if (error)
        throw glyph_load_failed(m_filename, char_index);

    // create glyph
    m_glyph_map[key] = std::make_unique<glyph>();
    glyph& glyph = *m_glyph_map[key];
    glyph.glyph_index = char_index;
    glyph.advance.x = static_cast<float>(m_face->handle->glyph->advance.x >> 6);
    glyph.advance.y = static_cast<float>(m_face->handle->glyph->advance.y >> 6);

    // get the handle to the bitmap
    FT_Bitmap bitmap = m_face->handle->glyph->bitmap;
    glGenTextures(1, &glyph.texture_id);
    glBindTexture(GL_TEXTURE_2D, glyph.texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.width, bitmap.rows,
        0, GL_LUMINANCE, GL_UNSIGNED_BYTE, bitmap.buffer);

    // create quad for it
    vertex data[4];
    data[0].pos = glm::vec2(0, 0);
    data[1].pos = glm::vec2(0, bitmap.rows);
    data[2].pos = glm::vec2(bitmap.width, bitmap.rows);
    data[3].pos = glm::vec2(bitmap.width, 0);
    data[0].tex = glm::vec2(0, 0);
    data[1].tex = glm::vec2(0, 1);
    data[2].tex = glm::vec2(1, 1);
    data[3].tex = glm::vec2(1, 0);

    // create a batch
    glyph.quad_ptr = std::make_shared<vertex_batch_t>(GL_QUADS);
    glyph.quad_ptr->load(data, 4);
    glyph.offset.x = static_cast<float>(m_face->handle->glyph->bitmap_left);
    glyph.offset.y = static_cast<float>(m_face->handle->glyph->bitmap_top);

    return glyph;
}

void xd::font::render(const std::string& text, const font_style& style,
    xd::shader_program* shader, const glm::mat4& mvp, glm::vec2 *pos,
    bool actual_rendering)
{
    int load_flags = style.m_force_autohint ? FT_LOAD_FORCE_AUTOHINT : FT_LOAD_NO_HINTING;
    // check if we're rendering using this font or a linked font
    if (style.m_type && style.m_type->length() != 0) {
        font_map_t::iterator i = m_linked_fonts.find(*style.m_type);
        if (i == m_linked_fonts.end())
            throw invalid_font_type(*style.m_type);
        font_style linked_style = style;
        linked_style.m_type = std::nullopt;
        i->second->render(text, linked_style, shader, mvp, pos, actual_rendering);
        return;
    }

    glm::vec2 text_pos;
    if (pos) text_pos = *pos;

    // check if the font size is already loaded
    auto it = m_face->sizes.find(style.m_size);
    if (it == m_face->sizes.end()) {
        // load the size
        load_size(style.m_size, load_flags);
    } else {
        // activate the size
        FT_Activate_Size(it->second);
    }

    // Setup harfbuzz
    hb_buffer_add_utf8(m_face->hb_buffer, text.c_str(), text.length(), 0, text.length());
    unsigned int glyph_count = hb_buffer_get_length(m_face->hb_buffer);
    hb_glyph_info_t* hb_glyph_infos = hb_buffer_get_glyph_infos(m_face->hb_buffer, &glyph_count);
    hb_buffer_set_script(m_face->hb_buffer, ucdn_get_script(hb_glyph_infos[0].codepoint));
    hb_buffer_guess_segment_properties(m_face->hb_buffer);
    hb_shape(m_face->hb_font, m_face->hb_buffer, NULL, 0);
    hb_glyph_position_t *hb_glyph_positions = hb_buffer_get_glyph_positions(m_face->hb_buffer, &glyph_count);

    // is kerning supported
    FT_Bool kerning = FT_HAS_KERNING(m_face->handle);

    if (actual_rendering) {
        // setup the shader
        shader->use();
        shader->bind_uniform(m_mvp_uniform, mvp);
        shader->bind_uniform(m_color_uniform, style.m_color);
        shader->bind_uniform(m_texture_uniform, 0);
    }

    FT_UInt prev_codepoint = 0;

    // render each glyph in the string
    for (unsigned int i = 0; i < glyph_count; ++i) {
        // get the unicode code point
        utf8::uint32_t codepoint = hb_glyph_infos[i].codepoint;

        // check for kerning
        if (kerning && codepoint && prev_codepoint) {
            FT_Vector kerning_delta;
            FT_Get_Kerning(m_face->handle, prev_codepoint, codepoint, FT_KERNING_DEFAULT, &kerning_delta);
            text_pos.x += kerning_delta.x / 64.0f;
        }

        auto& hb_glyph_pos = hb_glyph_positions[i];

        if (actual_rendering) {
            // get the cached glyph, or cache if it is not yet cached
            const glyph& glyph = load_glyph(codepoint, style.m_size, load_flags);

            // bind the texture
            glBindTexture(GL_TEXTURE_2D, glyph.texture_id);

            // calculate exact offset
            glm::vec2 glyph_pos = text_pos;
            glyph_pos.x += (hb_glyph_pos.x_offset / 64.0f) + glyph.offset.x;
            glyph_pos.y += (hb_glyph_pos.y_offset / 64.0f) - glyph.offset.y;

            // add optional letter spacing
            glyph_pos.x += style.m_letter_spacing / 2;

            // if shadow is enabled, draw the shadow first
            if (style.m_shadow) {
                // calculate shadow position
                glm::vec2 shadow_pos = glyph_pos;
                shadow_pos.x += style.m_shadow->x;
                shadow_pos.y += style.m_shadow->y;

                // calculate shadow color
                glm::vec4 shadow_color = style.m_shadow->color;
                shadow_color.a *= style.m_color.a;

                // bind uniforms
                shader->bind_uniform(m_color_uniform, shadow_color);
                shader->bind_uniform(m_position_uniform, shadow_pos);

                // draw shadow
                glyph.quad_ptr->render();

                // restore the text color
                shader->bind_uniform(m_color_uniform, style.m_color);
            }

            // if outline is enabled, draw outline
            if (style.m_outline) {
                // calculate outline color
                glm::vec4 outline_color = style.m_outline->color;
                outline_color.a *= style.m_color.a;

                // bind color
                shader->bind_uniform(m_color_uniform, outline_color);

                // draw font multiple times times to draw outline (EXPENSIVE!)
                for (int x = -style.m_outline->width; x <= style.m_outline->width; x++) {
                    for (int y = -style.m_outline->width; y <= style.m_outline->width; y++) {
                        if (x == 0 && y == 0) continue;
                        shader->bind_uniform(m_position_uniform, glyph_pos + glm::vec2(x, y));
                        glyph.quad_ptr->render();
                    }
                }

                // restore the text color
                shader->bind_uniform(m_color_uniform, style.m_color);
            }

            // bind uniforms
            shader->bind_uniform(m_position_uniform, glyph_pos);

            // draw the glyph
            glyph.quad_ptr->render();
        }

        // advance the position
        text_pos.x += hb_glyph_pos.x_advance / 64.0f;
        text_pos.y -= hb_glyph_pos.y_advance / 64.0f;
        text_pos.x += style.m_letter_spacing;

        // keep track of previous glyph to do kerning
        prev_codepoint = codepoint;
    }

    // Clear buffer
    hb_buffer_clear_contents(m_face->hb_buffer);

    // give back the updated coords
    if (pos)
        *pos = text_pos;
}

float xd::font::get_width(const std::string& text, const font_style& style)
{
    glm::vec2 pos(0.0f, 0.0f);
    render(text, style, nullptr, glm::mat4(), &pos, false);
    return pos.x;
}

const std::string& xd::font::get_mvp_uniform()
{
    return m_mvp_uniform;
}

void xd::font::set_mvp_uniform(const std::string& uniform_name)
{
    m_mvp_uniform = uniform_name;
}

const std::string& xd::font::get_pos_uniform()
{
    return m_position_uniform;
}

void xd::font::set_pos_uniform(const std::string& uniform_name)
{
    m_position_uniform = uniform_name;
}

const std::string& xd::font::get_color_uniform()
{
    return m_color_uniform;
}

void xd::font::set_color_uniform(const std::string& uniform_name)
{
    m_color_uniform = uniform_name;
}

const std::string& xd::font::get_texture_uniform()
{
    return m_texture_uniform;
}

void xd::font::set_texture_uniform(const std::string& uniform_name)
{
    m_texture_uniform = uniform_name;
}
