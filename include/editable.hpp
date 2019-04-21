#ifndef HPP_EDITABLE
#define HPP_EDITABLE

#include <memory>

class Property_Mapper;

// An editable object displayed in the map editor
// e.g. Map, Layer or Map_Object
class Editable {
public:
    void set_property_mapper(std::unique_ptr<Property_Mapper> property_mapper) {
        mapper = std::move(property_mapper);
    }
    Property_Mapper* get_property_mapper() {
        return mapper.get();
    }
    virtual ~Editable() = 0;
private:
    std::unique_ptr<Property_Mapper> mapper;
};

class QtTreePropertyBrowser;
class QtVariantPropertyManager;
class QtProperty;

// Abstract class to map QT property editor actions to object
class Property_Mapper {
public:
    virtual void populate(QtTreePropertyBrowser* browser,
            QtVariantPropertyManager* manager) = 0;
    virtual void change_property(QtProperty* prop) = 0;
    virtual ~Property_Mapper() = 0;
};

#endif
