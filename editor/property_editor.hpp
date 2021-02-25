#ifndef PROPERTY_EDITOR_HPP
#define PROPERTY_EDITOR_HPP

#include <QString>
#include "qtpropertybrowser/src/qttreepropertybrowser.h"
#include "qtpropertybrowser/src/qtvariantproperty.h"

class Editable;
class Map;
struct Layer;
class Game;
class Map_Object;
class QtVariantPropertyManager;
class QtVariantEditorFactory;

class Property_Editor : public QtTreePropertyBrowser {
    Q_OBJECT
public:
    Property_Editor();
    void set_map(Game& game, Map* map);
    void set_layer(Game& game, Layer* layer);
    void set_map_object(Game& game, Map_Object* obj);
    void reset_object() { current_object = nullptr; }
    void update_property(const QString& name, const QVariant& value);
private:
    void change_property(QtProperty* item);
    Editable* current_object;
    QtVariantPropertyManager* manager;
    QtVariantEditorFactory* factory;
};

#endif // PROPERTY_EDITOR_HPP
