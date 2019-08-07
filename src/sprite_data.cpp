#include "../include/sprite_data.hpp"
#include "../include/exceptions.hpp"
#include "../include/utility.hpp"
#include <iostream>
#include "../include/xd/system.hpp"
#include "../include/xd/asset_manager.hpp"

Sprite_Data::Sprite_Data(xd::asset_manager& manager) : asset_manager(manager) {}

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

std::unique_ptr<Sprite_Data> Sprite_Data::load(xd::asset_manager& manager, const std::string& filename) {
    rapidxml::memory_pool<> pool;
    char* content = pool.allocate_string(read_file(filename).c_str());
    rapidxml::xml_document<> doc;
    doc.parse<0>(content);
    auto sprite_node = doc.first_node("Sprite");
    if (!sprite_node)
        throw xml_exception("Invalid sprite data file. Missing spritedata node");
    auto sprite_data = load(manager, *sprite_node);
    sprite_data->filename = filename;
    normalize_slashes(sprite_data->filename);
    return sprite_data;
}

std::unique_ptr<Sprite_Data> Sprite_Data::load(xd::asset_manager& manager, rapidxml::xml_node<>& node) {
    using boost::lexical_cast;

    std::unique_ptr<Sprite_Data> sprite_ptr(new Sprite_Data(manager));

    // Image and transparent color
    bool image_loaded = false;
    bool pose_images_loaded = true;
    bool frame_images_loaded = true;

    auto trans_color_attr = node.first_attribute("Transparent-Color");
    if (trans_color_attr)
        sprite_ptr->transparent_color = hex_to_color(trans_color_attr->value());

    if (auto attr = node.first_attribute("Image")) {
        image_loaded = true;
        std::string image_file = attr->value();
        normalize_slashes(image_file);
        sprite_ptr->image = manager.load_persistent<xd::texture>(
            image_file, sprite_ptr->transparent_color,
            GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
    }

    // Poses
    for (auto pose_node = node.first_node("Pose");
            pose_node; pose_node = pose_node->next_sibling("Pose")) {
        Pose pose;
        if (auto bb_node = pose_node->first_node("Bounding-Box")) {
            xd::rect rect;
            rect.x = lexical_cast<float>(bb_node->first_attribute("X")->value());
            rect.y = lexical_cast<float>(bb_node->first_attribute("Y")->value());
            rect.w = lexical_cast<float>(bb_node->first_attribute("Width")->value());
            rect.h = lexical_cast<float>(bb_node->first_attribute("Height")->value());
            if (rect.x > 0) pose.bounding_box.x = rect.x;
            if (rect.y > 0) pose.bounding_box.y = rect.y;
            if (rect.w > 0) pose.bounding_box.w = rect.w;
            if (rect.h > 0) pose.bounding_box.h = rect.h;
        }
        if (auto attr = pose_node->first_attribute("Duration"))
            pose.duration = lexical_cast<int>(attr->value());

        if (auto attr = pose_node->first_attribute("Repeats"))
            pose.repeats = lexical_cast<int>(attr->value());

        if (auto attr = pose_node->first_attribute("Require-Completion"))
            pose.require_completion = attr->value() == std::string("true");

        if (auto attr = pose_node->first_attribute("X-Origin"))
            pose.origin.x = lexical_cast<float>(attr->value());
        if (auto attr = pose_node->first_attribute("Y-Origin"))
            pose.origin.y = lexical_cast<float>(attr->value());

        // Pose image and transparent color
        if (auto attr = pose_node->first_attribute("Transparent-Color")) {
            pose.transparent_color = hex_to_color(attr->value());
            trans_color_attr = attr;
        } else if (trans_color_attr)
            pose.transparent_color = sprite_ptr->transparent_color;

        if (auto attr = pose_node->first_attribute("Image")) {
            std::string pose_image_file = attr->value();
            normalize_slashes(pose_image_file);
            pose.image = manager.load_persistent<xd::texture>(
                pose_image_file,
                pose.transparent_color,
                GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
        } else {
            pose_images_loaded = false;
        }

        // Frames
        for (auto frame_node = pose_node->first_node("Frame");
                frame_node; frame_node = frame_node->next_sibling("Frame")) {
            Frame frame;
            if (auto attr = frame_node->first_attribute("Duration"))
                frame.duration = lexical_cast<int>(attr->value());

            if (auto node = frame_node->first_node("Rectangle")) {
                frame.rectangle.x = lexical_cast<float>(node->first_attribute("X")->value());
                frame.rectangle.y  = lexical_cast<float>(node->first_attribute("Y")->value());
                frame.rectangle.w  = lexical_cast<float>(node->first_attribute("Width")->value());
                frame.rectangle.h  = lexical_cast<float>(node->first_attribute("Height")->value());
            }

            if (auto attr = frame_node->first_attribute("X-Mag"))
                frame.magnification.x = lexical_cast<float>(attr->value());
            if (auto attr = frame_node->first_attribute("Y-Mag"))
                frame.magnification.y = lexical_cast<float>(attr->value());

            if (auto attr = frame_node->first_attribute("Angle"))
                frame.angle = lexical_cast<int>(attr->value());

            if (auto attr = frame_node->first_attribute("Opacity"))
                frame.opacity = lexical_cast<float>(attr->value());

            if (auto attr = frame_node->first_attribute("Tween"))
                frame.tween_frame = attr->value() == std::string("true");

            // Frame image and transparent color
            if (auto attr = frame_node->first_attribute("Transparent-Color"))
                frame.transparent_color = hex_to_color(attr->value());
            else if (trans_color_attr)
                frame.transparent_color = sprite_ptr->transparent_color;

            if (auto attr = frame_node->first_attribute("Image")) {
                std::string frame_image_file = attr->value();
                normalize_slashes(frame_image_file);
                frame.image = manager.load_persistent<xd::texture>(
                    frame_image_file,
                    frame.transparent_color,
                    GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
            } else {
                frame_images_loaded = false;
            }

            // Sound effect
            if (auto attr = frame_node->first_attribute("Sound")) {
                frame.sound_file = attr->value();
            }

            pose.frames.push_back(frame);
        }

        if (pose.frames.empty()) {
            throw xml_exception("Invalid sprite data file. "
                "Pose must have at least one frame.");
        }

        sprite_ptr->poses.push_back(pose);
        int pose_index = sprite_ptr->poses.size() - 1;

        // Tags
        for (auto tag_node = pose_node->first_node("Tag");
                tag_node; tag_node = tag_node->next_sibling("Tag")) {
            std::string key = tag_node->first_attribute("Key")->value();
            std::string value = tag_node->first_attribute("Value")->value();
            capitalize(key);
            capitalize(value);
            sprite_ptr->poses[pose_index].tags[key] = value;
        }
    }
    if (sprite_ptr->poses.empty())
        throw xml_exception("Invalid sprite data file. Missing poses.");
    if (!image_loaded && !pose_images_loaded && !frame_images_loaded)
        throw xml_exception("Invalid sprite data file. Missing image.");

    return sprite_ptr;
}
