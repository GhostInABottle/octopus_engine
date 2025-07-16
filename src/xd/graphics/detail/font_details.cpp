#include "font_details.hpp"
#include "../exceptions.hpp"
#include "../../vendor/unicode_data.hpp"
#include "../../../log.hpp"

namespace xd::detail::font {

    ft_lib::ft_lib() : m_library(nullptr) {
        int error = FT_Init_FreeType(&m_library);
        if (error) throw freetype_init_failed();
    }
    ft_lib::~ft_lib() {
        FT_Done_FreeType(m_library);
    }

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

    face::face(std::shared_ptr<ft_lib> lib, const std::string& filename, std::unique_ptr<std::istream> stream)
        : library_ptr(lib)
        , stream_rec{ std::make_unique<FT_StreamRec>() }
        , istream{ std::move(stream) } {
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

        // load the font
        auto error = FT_Open_Face(*library_ptr, &args, 0, &handle);
        if (error) throw font_load_failed(filename);

        hb_font = hb_ft_font_create_referenced(handle);
        hb_buffer = hb_buffer_create();
        hb_buffer_set_content_type(hb_buffer, HB_BUFFER_CONTENT_TYPE_UNICODE);
    }

    face::~face() {
        hb_buffer_destroy(hb_buffer);
        hb_font_destroy(hb_font);
        FT_Done_Face(handle);
    }

    static const hb_script_t ucdn_script_translate[] = {
        HB_SCRIPT_COMMON,
        HB_SCRIPT_LATIN,
        HB_SCRIPT_GREEK,
        HB_SCRIPT_CYRILLIC,
        HB_SCRIPT_ARMENIAN,
        HB_SCRIPT_HEBREW,
        HB_SCRIPT_ARABIC,
        HB_SCRIPT_SYRIAC,
        HB_SCRIPT_THAANA,
        HB_SCRIPT_DEVANAGARI,
        HB_SCRIPT_BENGALI,
        HB_SCRIPT_GURMUKHI,
        HB_SCRIPT_GUJARATI,
        HB_SCRIPT_ORIYA,
        HB_SCRIPT_TAMIL,
        HB_SCRIPT_TELUGU,
        HB_SCRIPT_KANNADA,
        HB_SCRIPT_MALAYALAM,
        HB_SCRIPT_SINHALA,
        HB_SCRIPT_THAI,
        HB_SCRIPT_LAO,
        HB_SCRIPT_TIBETAN,
        HB_SCRIPT_MYANMAR,
        HB_SCRIPT_GEORGIAN,
        HB_SCRIPT_HANGUL,
        HB_SCRIPT_ETHIOPIC,
        HB_SCRIPT_CHEROKEE,
        HB_SCRIPT_CANADIAN_SYLLABICS,
        HB_SCRIPT_OGHAM,
        HB_SCRIPT_RUNIC,
        HB_SCRIPT_KHMER,
        HB_SCRIPT_MONGOLIAN,
        HB_SCRIPT_HIRAGANA,
        HB_SCRIPT_KATAKANA,
        HB_SCRIPT_BOPOMOFO,
        HB_SCRIPT_HAN,
        HB_SCRIPT_YI,
        HB_SCRIPT_OLD_ITALIC,
        HB_SCRIPT_GOTHIC,
        HB_SCRIPT_DESERET,
        HB_SCRIPT_INHERITED,
        HB_SCRIPT_TAGALOG,
        HB_SCRIPT_HANUNOO,
        HB_SCRIPT_BUHID,
        HB_SCRIPT_TAGBANWA,
        HB_SCRIPT_LIMBU,
        HB_SCRIPT_TAI_LE,
        HB_SCRIPT_LINEAR_B,
        HB_SCRIPT_UGARITIC,
        HB_SCRIPT_SHAVIAN,
        HB_SCRIPT_OSMANYA,
        HB_SCRIPT_CYPRIOT,
        HB_SCRIPT_BRAILLE,
        HB_SCRIPT_BUGINESE,
        HB_SCRIPT_COPTIC,
        HB_SCRIPT_NEW_TAI_LUE,
        HB_SCRIPT_GLAGOLITIC,
        HB_SCRIPT_TIFINAGH,
        HB_SCRIPT_SYLOTI_NAGRI,
        HB_SCRIPT_OLD_PERSIAN,
        HB_SCRIPT_KHAROSHTHI,
        HB_SCRIPT_BALINESE,
        HB_SCRIPT_CUNEIFORM,
        HB_SCRIPT_PHOENICIAN,
        HB_SCRIPT_PHAGS_PA,
        HB_SCRIPT_NKO,
        HB_SCRIPT_SUNDANESE,
        HB_SCRIPT_LEPCHA,
        HB_SCRIPT_OL_CHIKI,
        HB_SCRIPT_VAI,
        HB_SCRIPT_SAURASHTRA,
        HB_SCRIPT_KAYAH_LI,
        HB_SCRIPT_REJANG,
        HB_SCRIPT_LYCIAN,
        HB_SCRIPT_CARIAN,
        HB_SCRIPT_LYDIAN,
        HB_SCRIPT_CHAM,
        HB_SCRIPT_TAI_THAM,
        HB_SCRIPT_TAI_VIET,
        HB_SCRIPT_AVESTAN,
        HB_SCRIPT_EGYPTIAN_HIEROGLYPHS,
        HB_SCRIPT_SAMARITAN,
        HB_SCRIPT_LISU,
        HB_SCRIPT_BAMUM,
        HB_SCRIPT_JAVANESE,
        HB_SCRIPT_MEETEI_MAYEK,
        HB_SCRIPT_IMPERIAL_ARAMAIC,
        HB_SCRIPT_OLD_SOUTH_ARABIAN,
        HB_SCRIPT_INSCRIPTIONAL_PARTHIAN,
        HB_SCRIPT_INSCRIPTIONAL_PAHLAVI,
        HB_SCRIPT_OLD_TURKIC,
        HB_SCRIPT_KAITHI,
        HB_SCRIPT_BATAK,
        HB_SCRIPT_BRAHMI,
        HB_SCRIPT_MANDAIC,
        HB_SCRIPT_CHAKMA,
        HB_SCRIPT_MEROITIC_CURSIVE,
        HB_SCRIPT_MEROITIC_HIEROGLYPHS,
        HB_SCRIPT_MIAO,
        HB_SCRIPT_SHARADA,
        HB_SCRIPT_SORA_SOMPENG,
        HB_SCRIPT_TAKRI,
        HB_SCRIPT_UNKNOWN,
        HB_SCRIPT_BASSA_VAH,
        HB_SCRIPT_CAUCASIAN_ALBANIAN,
        HB_SCRIPT_DUPLOYAN,
        HB_SCRIPT_ELBASAN,
        HB_SCRIPT_GRANTHA,
        HB_SCRIPT_KHOJKI,
        HB_SCRIPT_KHUDAWADI,
        HB_SCRIPT_LINEAR_A,
        HB_SCRIPT_MAHAJANI,
        HB_SCRIPT_MANICHAEAN,
        HB_SCRIPT_MENDE_KIKAKUI,
        HB_SCRIPT_MODI,
        HB_SCRIPT_MRO,
        HB_SCRIPT_NABATAEAN,
        HB_SCRIPT_OLD_NORTH_ARABIAN,
        HB_SCRIPT_OLD_PERMIC,
        HB_SCRIPT_PAHAWH_HMONG,
        HB_SCRIPT_PALMYRENE,
        HB_SCRIPT_PAU_CIN_HAU,
        HB_SCRIPT_PSALTER_PAHLAVI,
        HB_SCRIPT_SIDDHAM,
        HB_SCRIPT_TIRHUTA,
        HB_SCRIPT_WARANG_CITI,
    };

    static const UCDRecord* get_ucd_record(uint32_t code) {
        int index, offset;

        if (code >= 0x110000)
            index = 0;
        else {
            index = index0[code >> (SHIFT1 + SHIFT2)] << SHIFT1;
            offset = (code >> SHIFT2) & ((1 << SHIFT1) - 1);
            index = index1[index + offset] << SHIFT2;
            offset = code & ((1 << SHIFT2) - 1);
            index = index2[index + offset];
        }

        return &ucd_records[index];
    }

    hb_script_t ucdn_get_script(hb_codepoint_t codepoint) {

        return ucdn_script_translate[get_ucd_record(codepoint)->script];
    }
}