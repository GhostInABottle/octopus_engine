#include "../include/exceptions.hpp"
#include "../include/sprite_data.hpp"
#include "../include/utility/color.hpp"
#include "../include/utility/direction.hpp"
#include "../include/utility/file.hpp"
#include "../include/utility/string.hpp"
#include "../include/xd/asset_manager.hpp"
#include "../include/xd/audio.hpp"
#include <iostream>

namespace detail {
    static std::shared_ptr<xd::texture> load_sprite_texture(xd::asset_manager& manager, std::string filename, xd::vec4 transparent_color) {
        auto fs = file_utilities::game_data_filesystem();
        auto stream = fs->open_binary_ifstream(filename);
        if (!stream || !*stream) {
            throw file_loading_exception{ "Failed to load sprite image " + filename };
        }

        return manager.load_persistent<xd::texture>(filename, *stream, transparent_color);
    }
}

Sprite_Data::Sprite_Data(xd::asset_manager& manager) : asset_manager(manager), has_diagonal_directions(false) {}

Sprite_Data::~Sprite_Data() {
    // Release persistent textures from manager
    if (image)
        asset_manager.release<xd::texture>(image);
    for (auto& pose : poses) {
        if (pose.image)
            asset_manager.release<xd::texture>(pose.image);
        for (auto& frame : pose.frames) {
            if (frame.image)
                asset_manager.release<xd::texture>(frame.image);
        }
    }
}

std::unique_ptr<Sprite_Data> Sprite_Data::load(const std::string& filename, xd::asset_manager& manager,
        xd::audio* audio, channel_group_type channel_group) {
    try {
        auto doc = std::make_unique<rapidxml::xml_document<>>();
        auto fs = file_utilities::game_data_filesystem();
        auto content = doc->allocate_string(fs->read_file(filename).c_str());
        doc->parse<0>(content);

        auto sprite_node = doc->first_node("Sprite");
        if (!sprite_node) {
            throw xml_exception("Missing Sprite node.");
        }

        auto sprite_data = load(*sprite_node, manager, audio, channel_group);

        sprite_data->filename = filename;
        string_utilities::normalize_slashes(sprite_data->filename);

        return sprite_data;
    } catch (std::exception& ex) {
        throw xml_exception("Error reading sprite data file " + filename + ": " + ex.what());
    }
}

std::unique_ptr<Sprite_Data> Sprite_Data::load(rapidxml::xml_node<>& node, xd::asset_manager& manager,
        xd::audio* audio, channel_group_type channel_group) {
    auto sprite_ptr = std::make_unique<Sprite_Data>(manager);
    // Image and transparent color
    bool image_loaded = false;
    bool pose_images_loaded = true;
    bool frame_images_loaded = true;

    auto trans_color_attr = node.first_attribute("Transparent-Color");
    if (trans_color_attr)
        sprite_ptr->transparent_color = hex_to_color(trans_color_attr->value());

    if (auto attr = node.first_attribute("Image")) {
        std::string image_file = attr->value();
        sprite_ptr->image = detail::load_sprite_texture(manager, image_file,
            sprite_ptr->transparent_color);
        image_loaded = true;
    }

    // Default sprite pose
    if (auto default_attr = node.first_attribute("Default-Pose")) {
        sprite_ptr->default_pose = default_attr->value();
        string_utilities::capitalize(sprite_ptr->default_pose);
    }

    bool default_pose_found = false;
    // Poses
    for (auto pose_node = node.first_node("Pose");
            pose_node; pose_node = pose_node->next_sibling("Pose")) {
        Pose pose;

        // Bounding box
        if (auto bb_node = pose_node->first_node("Bounding-Box")) {
            xd::rect rect;
            rect.x = std::stof(bb_node->first_attribute("X")->value());
            rect.y = std::stof(bb_node->first_attribute("Y")->value());
            rect.w = std::stof(bb_node->first_attribute("Width")->value());
            rect.h = std::stof(bb_node->first_attribute("Height")->value());
            pose.bounding_box.x = rect.x;
            pose.bounding_box.y = rect.y;
            if (rect.w > 0) pose.bounding_box.w = rect.w;
            if (rect.h > 0) pose.bounding_box.h = rect.h;
        }

        // Bounding circle
        if (auto node = pose_node->first_node("Bounding-Circle")) {
            auto x = std::stof(node->first_attribute("X")->value());
            auto y = std::stof(node->first_attribute("Y")->value());
            auto radius = std::stof(node->first_attribute("Radius")->value());
            pose.bounding_circle = xd::circle{x, y, radius};
            pose.bounding_box = static_cast<xd::rect>(pose.bounding_circle.value());
        }

        // Animation properties
        if (auto attr = pose_node->first_attribute("Duration"))
            pose.duration = std::stoi(attr->value());

        if (auto attr = pose_node->first_attribute("Repeats"))
            pose.repeats = std::stoi(attr->value());

        if (auto attr = pose_node->first_attribute("Require-Completion"))
            pose.require_completion = string_utilities::string_to_bool(attr->value());

        if (auto attr = pose_node->first_attribute("X-Origin"))
            pose.origin.x = std::stof(attr->value());
        if (auto attr = pose_node->first_attribute("Y-Origin"))
            pose.origin.y = std::stof(attr->value());

        // Pose image and transparent color
        if (auto attr = pose_node->first_attribute("Transparent-Color")) {
            pose.transparent_color = hex_to_color(attr->value());
            trans_color_attr = attr;
        } else if (trans_color_attr)
            pose.transparent_color = sprite_ptr->transparent_color;

        if (auto attr = pose_node->first_attribute("Image")) {
            std::string pose_image_file = attr->value();
            pose.image = detail::load_sprite_texture(manager, pose_image_file,
                pose.transparent_color);
        } else {
            pose_images_loaded = false;
        }

        // Frames
        for (auto frame_node = pose_node->first_node("Frame");
                frame_node; frame_node = frame_node->next_sibling("Frame")) {
            Frame frame;
            if (auto attr = frame_node->first_attribute("Duration")) {
                frame.duration = std::stoi(attr->value());
            }
            if (auto attr = frame_node->first_attribute("Max-Duration")) {
                frame.max_duration = std::stoi(attr->value());
            }

            // Source rectangle
            if (auto node = frame_node->first_node("Rectangle")) {
                frame.rectangle.x = std::stof(node->first_attribute("X")->value());
                frame.rectangle.y  = std::stof(node->first_attribute("Y")->value());
                frame.rectangle.w  = std::stof(node->first_attribute("Width")->value());
                frame.rectangle.h  = std::stof(node->first_attribute("Height")->value());
            }

            // Frame properties
            if (auto attr = frame_node->first_attribute("X-Mag"))
                frame.magnification.x = std::stof(attr->value());
            if (auto attr = frame_node->first_attribute("Y-Mag"))
                frame.magnification.y = std::stof(attr->value());

            if (auto attr = frame_node->first_attribute("Angle"))
                frame.angle = std::stoi(attr->value());

            if (auto attr = frame_node->first_attribute("Opacity"))
                frame.opacity = std::stof(attr->value());

            if (auto attr = frame_node->first_attribute("Tween"))
                frame.tween_frame = string_utilities::string_to_bool(attr->value());

            // Frame image and transparent color
            if (auto attr = frame_node->first_attribute("Transparent-Color"))
                frame.transparent_color = hex_to_color(attr->value());
            else if (trans_color_attr)
                frame.transparent_color = sprite_ptr->transparent_color;

            if (auto attr = frame_node->first_attribute("Image")) {
                std::string frame_image_file = attr->value();
                frame.image = detail::load_sprite_texture(manager, frame_image_file,
                    frame.transparent_color);
            } else {
                frame_images_loaded = false;
            }

            // Sound effect
            if (audio) {
                auto fs = file_utilities::game_data_filesystem();
                if (auto sound_file_attr = frame_node->first_attribute("Sound")) {
                    auto sound_filename = std::string{ sound_file_attr->value() };
                    frame.sound_file = std::make_shared<xd::sound>(*audio, sound_filename,
                        fs->open_binary_ifstream(sound_filename), channel_group);
                }

                if (auto node = frame_node->first_node("Sound")) {
                    if (frame.sound_file) {
                        throw xml_exception("Both frame sound attribute and node are defined for " + frame.sound_file->get_filename());
                    }

                    if (auto sound_file_attr = node->first_attribute("Filename")) {
                        auto sound_filename = std::string{ sound_file_attr->value() };
                        frame.sound_file = std::make_shared<xd::sound>(*audio, sound_filename,
                            fs->open_binary_ifstream(sound_filename), channel_group);
                    } else {
                        throw xml_exception("Frame has a sound node but the filename is missing");
                    }

                    if (auto pitch_attr = node->first_attribute("Pitch")) {
                        auto pitch = std::stof(pitch_attr->value());
                        frame.sound_file->set_pitch(pitch);
                    }

                    if (auto volume_attr = node->first_attribute("Volume")) {
                        auto volume = std::stof(volume_attr->value());
                        frame.sound_file->set_volume(volume);
                        frame.sound_volume = volume;
                    }
                }
            }

            pose.frames.push_back(frame);
        }

        if (pose.frames.empty()) {
            throw xml_exception("Invalid sprite data file. "
                "Pose must have at least one frame.");
        }

        sprite_ptr->poses.push_back(pose);
        int pose_index = sprite_ptr->poses.size() - 1;

        // Pose tags
        for (auto tag_node = pose_node->first_node("Tag");
                tag_node; tag_node = tag_node->next_sibling("Tag")) {
            std::string key = tag_node->first_attribute("Key")->value();
            std::string value = tag_node->first_attribute("Value")->value();
            string_utilities::capitalize(key);
            string_utilities::capitalize(value);
            if (key == "NAME") {
                sprite_ptr->poses[pose_index].name = value;
                if (value == sprite_ptr->default_pose) {
                    default_pose_found = true;
                }
            } else if (key == "STATE") {
                sprite_ptr->poses[pose_index].state = value;
            } else if (key == "DIRECTION") {
                auto dir = string_to_direction(value);
                sprite_ptr->poses[pose_index].direction = dir;
                if (!sprite_ptr->has_diagonal_directions) {
                    sprite_ptr->has_diagonal_directions = is_diagonal(dir);
                }
            } else {
                throw tmx_exception("Unsupported pose tag " + key + " with value " + value);
            }
        }
    }

    if (!default_pose_found && sprite_ptr->default_pose != "") {
        throw tmx_exception("Could not find default pose " + sprite_ptr->default_pose +
            " when loading " + sprite_ptr->filename);
    }

    if (sprite_ptr->poses.empty())
        throw xml_exception("Invalid sprite data file. Missing poses.");

    if (!image_loaded && !pose_images_loaded && !frame_images_loaded)
        throw xml_exception("Invalid sprite data file. Missing image.");

    return sprite_ptr;
}
