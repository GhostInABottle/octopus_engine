#ifndef IMAGE_LAYER_MAPPER_HPP
#define IMAGE_LAYER_MAPPER_HPP

#include "layer_mapper.hpp"

class Game;
struct Image_Layer;

class Image_Layer_Mapper : public Layer_Mapper {
public:
    Image_Layer_Mapper(Game& game, Image_Layer* layer);
    void populate(QtTreePropertyBrowser* browser,
            QtVariantPropertyManager* manager);
    void change_property(QtProperty* prop);
private:
    Game& game;
    Image_Layer* layer;
};

#endif // IMAGE_LAYER_MAPPER_HPP
