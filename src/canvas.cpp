#include <stdexcept>
#include <regex>
#include <unordered_map>
#include <xd/graphics/stock_text_formatter.hpp>
#include <xd/graphics/simple_text_renderer.hpp>
#include <xd/graphics/framebuffer.hpp>
#include "../include/canvas.hpp"
#include "../include/utility.hpp"
#include "../include/game.hpp"
#include "../include/map.hpp"
#include "../include/sprite_data.hpp"
#include "../include/configurations.hpp"
#include "../include/log.hpp"

Canvas::Canvas(Game& game, const std::string& sprite, const std::string& pose_name, xd::vec2 position) :
        position(position), origin(0.5f, 0.5f), magnification(1.0f, 1.0f),
        angle(0.0f), opacity(1.0f), visible(false) {
    set_sprite(game, game.get_map()->get_asset_manager(), sprite, pose_name);
}

Canvas::Canvas(const std::string& filename, xd::vec2 position) :
        position(position), origin(0.5f, 0.5f), magnification(1.0f, 1.0f),
        angle(0.0f), opacity(1.0f), visible(false) {
    set_image(filename);
}

Canvas::Canvas(const std::string& filename, xd::vec2 position, xd::vec4 trans) :
        position(position), origin(0.5f, 0.5f), magnification(1.0f, 1.0f),
        angle(0.0f), opacity(1.0f), visible(false) {
    set_image(filename, trans);
}

Canvas::Canvas(Game& game, xd::vec2 position, const std::string& text, bool camera_relative) :
        position(position), text_renderer(&game.get_text_renderer()),
        font(game.get_font()), opacity(1.0f), visible(false),
        formatter(xd::create<xd::stock_text_formatter>()),
        style(new xd::font_style(game.get_font_style())),
        camera_relative_text(camera_relative) {
    // Use image texture for FBO rendering, if supported
    if (xd::framebuffer::extension_supported()) {
        image_texture = xd::create<xd::texture>(game.game_width, game.game_height,
            nullptr,xd::vec4(0), GL_CLAMP, GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }
    set_text(text);
}

void Canvas::set_image(const std::string& filename, xd::vec4 trans) {
    this->filename = filename;
    image_texture = xd::create<xd::texture>(normalize_slashes(filename),
        trans, GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
}

void Canvas::set_sprite(Game& game, xd::asset_manager& manager, const std::string& filename, const std::string& pose_name) {
    this->filename = filename;
    sprite = xd::create<Sprite>(game, Sprite_Data::load(manager, filename));
    set_pose(pose_name, "", Direction::NONE);
}

void Canvas::set_text(const std::string& text) {
    if (this->text == text && !this->text.empty())
        return;
    this->text = text;
    // Split tags across multiple lines
    // e.g. "{a=b}x\ny{/a}" => "{a=b}x{/a}", "{a=b}y{/a}"
    text_lines = split(text, "\n");
    if (text_lines.size() > 1) {
        struct tag_info {
            bool open;
            std::string name;
            std::string value;
        };
        std::unordered_map<std::string, std::deque<int>> tags;
        std::vector<tag_info> tag_infos;
        for (unsigned int i = 0; i < text_lines.size(); i++) {
            std::string line = text_lines[i];
            std::string open_tags;
            for (auto tag : tag_infos) {
                if (tag.open) {
                    auto open_tag = "{" + tag.name;
                    if (!tag.value.empty())
                        open_tag += "=" + tag.value;
                    open_tag += "}";
                    open_tags += open_tag;
                }
            }
            text_lines[i] = open_tags + text_lines[i];
            static std::regex opening("\\{(\\w+)=?((\\w|,)+)?\\}");
            static std::regex closing("\\{/(\\w+)\\}");
            std::smatch open_results;
            auto start = line.cbegin();
            while (std::regex_search(start, line.cend(), open_results, opening)) {
                tag_info info;
                info.name = open_results[1].str();
                if (open_results.size() > 2)
                    info.value = open_results[2].str();
                info.open = true;
                tag_infos.push_back(info);
                tags[info.name].push_back(tag_infos.size() - 1);
                start += open_results.position() + 1;
            }
            std::smatch close_results;
            start = line.cbegin();
            while (std::regex_search(start, line.cend(), close_results, closing)) {
                auto close_tag = close_results[1].str();
                if (tags.find(close_tag) != tags.end()) {
                    auto& ids = tags[close_tag];
                    int id = ids[0];
                    tag_infos[id].open = false;
                    ids.pop_front();
                } else {
                    LOGGER_E << "Closing tag " + close_tag + " didn't match any open tags in line: " + line;
                }
                start += close_results.position() + 1;
            }
            for (auto tag = tag_infos.rbegin(); tag != tag_infos.rend(); ++tag) {
                if (tag->open)
                    text_lines[i] += "{/" + tag->name + "}";
            }
        }
    }
}

void Canvas::render_text(const std::string& text, float x, float y) {
    text_renderer->render_formatted(font, formatter, *style, x, y, text);
}

void Canvas::set_font(const std::string& font_file) {
    if (!file_exists(font_file))
        throw std::runtime_error("Couldn't read font file " + font_file);
    font = xd::create<xd::font>(font_file);
}

void Canvas::link_font(const std::string& type, const std::string& font_file) {
    if (file_exists(font_file)) {
        font->link_font(type, xd::create<xd::font>(font_file));
    } else {
        LOGGER_W << "Couldn't read '" << type << "' font file " << font_file;
    }
}
