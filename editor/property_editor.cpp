#include "property_editor.hpp"
#include "mappers/map_mapper.hpp"
#include "mappers/map_object_mapper.hpp"
#include "mappers/image_layer_mapper.hpp"
#include "mappers/object_layer_mapper.hpp"
#include "qtpropertybrowser/src/qtpropertymanager.h"
#include "qtpropertybrowser/src/qtvariantproperty.h"
#include "../src/map.hpp"
#include "../src/map_object.hpp"
#include "../src/image_layer.hpp"
#include "../src/object_layer.hpp"
#include <QMessageBox>

Property_Editor::Property_Editor()
        : QtTreePropertyBrowser(), current_object(nullptr) {
    manager = new QtVariantPropertyManager();
    factory = new QtVariantEditorFactory();
    setFactoryForManager(manager, factory);
    setPropertiesWithoutValueMarked(true);
    setRootIsDecorated(false);
    connect(manager, &QtVariantPropertyManager::propertyChanged,
        this, &Property_Editor::change_property);
}

void Property_Editor::set_map(Game&, Map* map) {
    current_object = nullptr;
    if (map) {
        std::unique_ptr<Map_Mapper> mapper(new Map_Mapper(map));
        mapper->populate(this, manager);
        map->set_property_mapper(std::move(mapper));
    }
    current_object = static_cast<Editable*>(map);
}

void Property_Editor::set_layer(Game& game, Layer* layer) {
    if (current_object) {
        current_object->set_property_mapper(nullptr);
        current_object = nullptr;
    }
    if (layer) {
        std::unique_ptr<Layer_Mapper> mapper;
        if (Image_Layer* img = dynamic_cast<Image_Layer*>(layer)) {
            mapper.reset(new Image_Layer_Mapper(game, img));
        } else if (Object_Layer* obj = dynamic_cast<Object_Layer*>(layer)) {
            mapper.reset(new Object_Layer_Mapper(obj));
        } else {
            mapper.reset(new Layer_Mapper(layer));
        }
        mapper->populate(this, manager);
        layer->set_property_mapper(std::move(mapper));
    }
    current_object = static_cast<Editable*>(layer);
}

void Property_Editor::set_map_object(Game& game, Map_Object* obj) {
    if (current_object) {
        current_object->set_property_mapper(nullptr);
        current_object = nullptr;
    }
    if (obj) {
        std::unique_ptr<Map_Object_Mapper> mapper(new Map_Object_Mapper(game, obj));
        mapper->populate(this, manager);
        obj->set_property_mapper(std::move(mapper));
    }
    current_object = static_cast<Editable*>(obj);
}

void Property_Editor::update_property(const QString& name, const QVariant& value) {
    for (auto& property : manager->properties()) {
        if (property->propertyName() == name) {
            manager->setValue(property, value);
            break;
        }
    }
}

void Property_Editor::change_property(QtProperty* prop) {
    if (!current_object)
        return;
    auto mapper = current_object->get_property_mapper();
    mapper->change_property(prop);
}
