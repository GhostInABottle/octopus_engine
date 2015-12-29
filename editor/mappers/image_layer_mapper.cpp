#include "image_layer_mapper.hpp"
#include "qtpropertybrowser/src/qttreepropertybrowser.h"
#include "../qtpropertybrowser/src/qtpropertymanager.h"
#include "../qtpropertybrowser/src/qtvariantproperty.h"
#include "../../include/image_layer.hpp"
#include "../../include/sprite_data.hpp"

Image_Layer_Mapper::Image_Layer_Mapper(Game& game, Image_Layer* layer) : Layer_Mapper(layer), game(game), layer(layer) {}

void Image_Layer_Mapper::populate(QtTreePropertyBrowser* browser, QtVariantPropertyManager* manager) {
    if (!layer)
        return;
    Layer_Mapper::populate(browser, manager);
    // Repeat
    QtVariantProperty* item = manager->addProperty(QVariant::Bool, "Repeat");
    item->setValue(layer->repeat);
    browser->addProperty(item);
    // Fixed
    item = manager->addProperty(QVariant::Bool, "Fixed");
    item->setValue(layer->fixed);
    browser->addProperty(item);
    // Velocity
    item = manager->addProperty(QVariant::PointF, "Velocity");
    float x = layer->velocity.x;
    float y = layer->velocity.y;
    item->setValue(QPointF(x, y));
    browser->addProperty(item);
    // Image
    item = manager->addProperty(QVariant::String, "Image");
    item->setValue(QString::fromStdString(layer->image_source));
    browser->addProperty(item);
    // Transparent Color
    item = manager->addProperty(QVariant::Color,  "Transparent");
    auto c = layer->image_trans_color;
    item->setValue(QColor::fromRgbF(c.r, c.g, c.b, c.a));
    browser->addProperty(item);
    // Sprite
    item = manager->addProperty(QVariant::String, "Sprite");
    if (layer->sprite)
        item->setValue(QString::fromStdString(layer->sprite->get_filename()));
    browser->addProperty(item);
    // Pose
    item = manager->addProperty(QVariant::String, "Pose");
    if (layer->sprite) {
        std::string pose_name = layer->sprite->get_pose().tags["NAME"];
        item->setValue(QString::fromStdString(pose_name));
    }
    browser->addProperty(item);
}

void Image_Layer_Mapper::change_property(QtProperty* prop) {
    if (!layer)
        return;
    Layer_Mapper::change_property(prop);
    QtVariantProperty* vprop = static_cast<QtVariantProperty*>(prop);
    QString prop_name = prop->propertyName();
    QVariant& prop_value = vprop->value();
    if (prop_name == "Repeat") {
        layer->repeat = prop_value.toBool();
    } else if (prop_name == "Fixed") {
        layer->fixed = prop_value.toBool();
    } else if (prop_name == "Velocity") {
        QPointF pos = prop_value.toPointF();
        layer->velocity = xd::vec2(pos.x(), pos.y());
    } else if (prop_name == "Image") {
        layer->set_image(prop_value.toString().toStdString());
    } else if (prop_name == "Transparent") {
        QColor c = prop_value.value<QColor>();
        layer->image_trans_color = xd::vec4(c.redF(), c.greenF(),
                                            c.blueF(), c.alphaF());
        layer->set_image(layer->image_source);
    } else if (prop_name == "Sprite") {
        layer->set_sprite(game, prop_value.toString().toStdString());
    } else if (prop_name == "Pose") {
        layer->set_pose(prop_value.toString().toStdString(), "", Direction::NONE);
    }
}
