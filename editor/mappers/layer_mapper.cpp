#include "layer_mapper.hpp"
#include "qtpropertybrowser/src/qttreepropertybrowser.h"
#include "../qtpropertybrowser/src/qtpropertymanager.h"
#include "../qtpropertybrowser/src/qtvariantproperty.h"
#include "../../src/map/layers/layer.hpp"

Layer_Mapper::Layer_Mapper(Layer* layer) : layer(layer) {}

void Layer_Mapper::populate(QtTreePropertyBrowser* browser, QtVariantPropertyManager* manager) {
    if (!layer)
        return;
    manager->clear();
    // Name
    QtVariantProperty* item = manager->addProperty(QVariant::String, "Name");
    item->setValue(QString::fromStdString(layer->name));
    browser->addProperty(item);
    // Opacity
    item = manager->addProperty(QVariant::Double, "Opacity");
    item->setValue(layer->opacity);
    item->setAttribute("singleStep", 0.01);
    item->setAttribute("decimals", 3);
    browser->addProperty(item);
    // Visible
    item = manager->addProperty(QVariant::Bool, "Visible");
    item->setValue(layer->visible);
    browser->addProperty(item);
    // Vertex Shader
    item = manager->addProperty(QVariant::String, "Vertex Shader");
    item->setValue(QString::fromStdString(layer->get_property("vertex-shader")));
    browser->addProperty(item);
    // Fragment Shader
    item = manager->addProperty(QVariant::String, "Fragment Shader");
    item->setValue(QString::fromStdString(layer->get_property("fragment-shader")));
    browser->addProperty(item);
}

void Layer_Mapper::change_property(QtProperty* prop) {
    if (!layer)
        return;
    QtVariantProperty* vprop = static_cast<QtVariantProperty*>(prop);
    QString prop_name = prop->propertyName();
    QVariant prop_value = vprop->value();
    if (prop_name == "Name") {
        layer->name = prop_value.toString().toStdString();
    } else if (prop_name == "Opacity") {
        layer->opacity = prop_value.toFloat();
    } else if (prop_name == "Visible") {
        layer->visible = prop_value.toBool();
    } else if (prop_name == "Vertex Shader") {
        layer->set_editor_property("vertex-shader", prop_value.toString().toStdString());
    } else if (prop_name == "Fragment Shader") {
        layer->set_editor_property("fragment-shader", prop_value.toString().toStdString());
    }
}
