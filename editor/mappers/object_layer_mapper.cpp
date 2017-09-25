#include "object_layer_mapper.hpp"
#include "qtpropertybrowser/src/qttreepropertybrowser.h"
#include "../qtpropertybrowser/src/qtpropertymanager.h"
#include "../qtpropertybrowser/src/qtvariantproperty.h"
#include "../../include/object_layer.hpp"

Object_Layer_Mapper::Object_Layer_Mapper(Object_Layer* layer) : Layer_Mapper(layer), layer(layer) {}

void Object_Layer_Mapper::populate(QtTreePropertyBrowser* browser, QtVariantPropertyManager* manager) {
    if (!layer)
        return;
    Layer_Mapper::populate(browser, manager);
    // Color
    QtVariantProperty* item = manager->addProperty(QVariant::Color,  "Color");
    auto c = layer->color;
    item->setValue(QColor::fromRgbF(c.r, c.g, c.b, c.a));
    browser->addProperty(item);
}

void Object_Layer_Mapper::change_property(QtProperty* prop) {
    if (!layer)
        return;
    Layer_Mapper::change_property(prop);
    QtVariantProperty* vprop = static_cast<QtVariantProperty*>(prop);
    QString prop_name = prop->propertyName();
    QVariant prop_value = vprop->value();
    if (prop_name == "Color") {
        QColor c = prop_value.value<QColor>();
        layer->color = xd::vec4(c.redF(), c.greenF(), c.blueF(), c.alphaF());
    }
}

