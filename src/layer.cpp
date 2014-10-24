#include "../include/layer.hpp"
#include "../include/layer_renderer.hpp"
#include "../include/layer_updater.hpp"
#include "../include/utility.hpp"
#include "../include/base64.hpp"
#include "../include/exceptions.hpp"
#include <zlib.h>
#include <boost/lexical_cast.hpp>

Layer::Layer() : width(0), height(0), opacity(1.0f), visible(true) {}

Layer::~Layer() {}
