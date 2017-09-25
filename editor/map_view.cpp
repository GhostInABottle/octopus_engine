#include "../include/game.hpp"
#include "../include/map.hpp"
#include "../include/map_object.hpp"
#include "../include/sprite.hpp"
#include "../include/layer.hpp"
#include "../include/camera.hpp"
#include "../include/configurations.hpp"
#include "../include/log.hpp"
#include "map_view.hpp"
#include <QMouseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <algorithm>

namespace detail {
    xd::rect get_object_box(Map_Object* obj) {
        xd::vec2 size;
        if (auto sprite = obj->get_sprite())
            size = sprite->get_size();
        else
            size = obj->get_size();
        return xd::rect(obj->get_position(), size);
    }
}

Map_View::Map_View(QWidget* parent) :
    QGLWidget(parent),
    initialized(false),
    scale_index(1),
    selected_object(nullptr),
    mouse_pressed(false)
{
    setFocusPolicy(Qt::StrongFocus);
}

void Map_View::load_map(const std::string& name) {
    if (name == map_name)
        return;
    selected_object = nullptr;
    map_name = name;
    try {
        game->load_map(map_name);
    } catch (std::exception& e) {
        QMessageBox::warning(this, tr(e.what()),
           tr(e.what()));
        return;
    }
    refresh_scrollbars();
    emit map_changed(map_name);
}

void Map_View::new_map() {
    selected_object = nullptr;
    map_name = "";
    game->new_map(xd::ivec2(40, 30), xd::ivec2(8));
    emit map_changed("");
}

void Map_View::save() {
    auto map = game->get_map();
    if (map->get_filename().empty())
        save_as();
    else
        map->save(map->get_filename());
}

void Map_View::save_as() {
    std::string filename = QFileDialog::getSaveFileName(this, tr("Save Map"),
            QString(), tr("Map Files (*.tmx)")).toStdString();
    if (!filename.empty()) {
        game->get_map()->save(filename);
        map_name = filename;
    }
}

void Map_View::open_map() {
    std::string filename = QFileDialog::getOpenFileName(this, tr("Open Map"),
            QString(), tr("Map Files (*.tmx)")).toStdString();
    if (!filename.empty())
        load_map(filename);
}

int Map_View::map_width() const {
    auto map = game->get_map();
    return map->get_width() * map->get_tile_width();
}

int Map_View::map_height() const {
    auto map = game->get_map();
    return map->get_height() * map->get_tile_height();
}

Game* Map_View::get_game() {
    return game.get();
}

Map* Map_View::get_map() {
    return game->get_map();
}

Map_Object* Map_View::get_object(const std::string& name) {
    return get_map()->get_object(name);
}

Layer* Map_View::get_layer(const std::string& name) {
    return get_map()->get_layer(name);
}

void Map_View::add_layer(Layer_Types type) {
    get_map()->add_layer(type);
}

void Map_View::delete_layer(const std::string& name) {
    selected_object = nullptr;
    get_map()->delete_layer(name);
}

void Map_View::add_object() {
    auto object = get_map()->add_object();
    object->set_size(xd::vec2(16, 16));
}

std::vector<std::string> Map_View::get_layer_names() {
    std::vector<std::string> layer_names;
    auto map = get_map();
    for (int i = 1; i <= map->layer_count(); ++i) {
        layer_names.push_back(map->get_layer(i)->name);
    }
    return layer_names;
}

std::vector<std::string> Map_View::get_object_names() {
    std::vector<std::string> object_names;
    auto objects = get_map()->get_objects();
    for (auto& obj : objects) {
        object_names.push_back(obj.second->get_name());
    }
    return object_names;
}

void Map_View::highlight_object(Map_Object* obj) {
    selected_object = obj;
}

void Map_View::initializeGL() {
    if (initialized)
        return;
    try {
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            throw xd::window_creation_failed();
        }
        Configurations::parse("config.ini");
        Game::game_width = 1;
        Game::game_height = 1;
        game.reset(new Game(true));
        new_map();
        tick_timer = new QElapsedTimer();
        tick_timer->start();
        draw_timer = new QTimer(this);
        connect(draw_timer, &QTimer::timeout, this, &Map_View::updateGL);
        draw_timer->start(16);
        update_timer = new QTimer(this);
        connect(update_timer, &QTimer::timeout, this, &Map_View::logic_update);
        update_timer->start(25);
        game->get_camera()->set_magnification(get_scale());
        emit resize(map_width(), map_height());
    } catch (std::exception& e) {
        QMessageBox::warning(this, tr(e.what()),
           tr(e.what()));
    }
}

void Map_View::paintGL() {
    // If map size changed, resize viewport and scroll bars
    if (get_map()->is_changed()) {
        refresh_scrollbars();
        resizeGL(width(), height());
    }
    // Render everything
    game->render();
    auto camera = game->get_camera();
    if (selected_object) {
        auto object_box = detail::get_object_box(selected_object);
        camera->draw_rect(object_box, xd::vec4(1.0f), false);
    }
    xd::rect map_border(0.1f, 0.0f, map_width() - 0.1f, map_height() - 0.1f);
    camera->draw_rect(map_border, xd::vec4(0.0f, 0.0f, 0.0f, 1.0f), false);
}

void Map_View::resizeGL(int width, int height) {
    Game::game_width = map_width();
    Game::game_height = map_height();
    game->set_size(width, height);
}

void Map_View::mousePressEvent(QMouseEvent* mouse_event) {
    if (mouse_event->button() == Qt::MouseButton::LeftButton) {
        mouse_pressed = true;
        selected_object = nullptr;
        press_offset = xd::vec2(0, 0);
        xd::rect viewport = game->get_camera()->get_viewport();
        xd::rect mouse_box(
            (mouse_event->x() - viewport.x) / get_scale(),
            (mouse_event->y() - viewport.y) / get_scale(),
            1, 1
        );
        auto objects = game->get_map()->get_objects();
        for (auto& object_pair : objects) {
            auto name = object_pair.second->get_name();
            auto obj = object_pair.second;
            xd::rect object_box = detail::get_object_box(obj.get());
            if (object_box.intersects(mouse_box)) {
                press_offset = xd::vec2(
                    mouse_box.x - object_box.x,
                    mouse_box.y - object_box.y);
                emit object_selected(name);
                return;
            }
        }
        emit object_selected("");
    }
}

void Map_View::mouseReleaseEvent(QMouseEvent* mouse_event) {
    if (mouse_event->button() == Qt::MouseButton::LeftButton) {
        mouse_pressed = false;
        if (selected_object) {
            xd::vec2 pos = selected_object->get_position();
            QPoint pt(static_cast<int>(pos.x), static_cast<int>(pos.y));
            emit object_dragged(pt);
        }
    }
}

void Map_View::mouseMoveEvent(QMouseEvent* mouse_event) {
    if (mouse_pressed && selected_object) {
        xd::rect viewport = game->get_camera()->get_viewport();
        float mouse_x = (mouse_event->x() - viewport.x) / get_scale();
        float mouse_y = (mouse_event->y() - viewport.y) / get_scale();
        xd::rect object_box = detail::get_object_box(selected_object);
        float distance = std::abs(mouse_x - object_box.x) + std::abs(mouse_y - object_box.y);
        if (distance > 2) {
            xd::vec2 pos(mouse_x - press_offset.x, mouse_y - press_offset.y);
            selected_object->set_position(pos);
        }
    }
}

void Map_View::keyPressEvent(QKeyEvent* event) {
    float old_scale_index = scale_index;
    if (event->key() == Qt::Key_Plus && scale_index < 4) {
        scale_index++;
    } else if (event->key() == Qt::Key_Minus && scale_index > 0) {
        scale_index--;
    } else {
        QGLWidget::keyPressEvent(event);
    }
    if (scale_index != old_scale_index) {
        refresh_scrollbars();
        resizeGL(width(), height());
    }
}

void Map_View::logic_update() {
    game->set_ticks(static_cast<int>(tick_timer->elapsed()));
    game->get_map()->update();
}

void Map_View::refresh_scrollbars() {
    game->get_camera()->set_magnification(get_scale());
    int min_width = static_cast<int>(map_width() * get_scale());
    int min_height = static_cast<int>(map_height() * get_scale());
    setMinimumSize(min_width, min_height);
}
