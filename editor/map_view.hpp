#ifndef MAP_VIEW_HPP
#define MAP_VIEW_HPP

#include <QOpenGLWidget>
#include <memory>
#include <QTimer>
#include <QElapsedTimer>
#include <string>
#include <vector>
#include "../include/xd/graphics/types.hpp"
#include "../include/layer_types.hpp"

class Game;
class Map;
class Map_Object;
struct Layer;
class QMouseEvent;

class Map_View : public QOpenGLWidget {
    Q_OBJECT
public:
    explicit Map_View(QWidget* parent = nullptr);
    void load_map(const std::string& name);
    Game* get_game();
    Map* get_map();
    Map_Object* get_object(const std::string& name);
    Layer* get_layer(const std::string& name);
    void add_layer(Layer_Type type);
    void delete_layer(const std::string& name);
    void add_object();
    std::vector<std::string> get_layer_names();
    std::vector<std::string> get_object_names();
    void highlight_object(Map_Object* obj);
    float get_scale() const { return scales[scale_index]; }
    void refresh_scrollbars();
protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent* mouse_event);
    void mouseReleaseEvent(QMouseEvent* mouse_event);
    void mouseMoveEvent(QMouseEvent* mouse_event);
    void keyPressEvent(QKeyEvent *event);
private:
    std::unique_ptr<Game> game;
    std::string map_name;
    bool initialized;
    QTimer* draw_timer;
    QTimer* update_timer;
    QElapsedTimer* tick_timer;
    int scale_index;
    constexpr static float scales[] = { 1.0f, 2.0f, 4.0f, 8.0f, 16.0f };
    Map_Object* selected_object;
    bool mouse_pressed;
    xd::vec2 press_offset;
signals:
    void map_changed(const std::string& filename);
    void object_selected(const std::string& object_name);
    void object_dragged(QPoint new_position);
public slots:
    void logic_update();
    void new_map();
    // Save current map
    void save();
    // Select file to save
    void save_as();
    // Show map dialog and load a map
    void open_map();
};

#endif // MAP_VIEW_HPP
