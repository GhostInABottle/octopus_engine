#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>
#include "map_view.hpp"

namespace Ui {
class Main_Window;
}

class Property_Editor;

class Main_Window : public QMainWindow {
    Q_OBJECT

public:
    explicit Main_Window(QWidget* parent = 0);
    ~Main_Window();
private slots:
    // Open a project folder
    void open_project();
    // Select map by double clicking on it
    void select_map(const QModelIndex& index);
    // Populate layer and object names for a new map
    void update_map(std::string filename);
    // Update object position
    void drag_object(QPoint new_position);
    // Select object from list
    void select_list_object(const QModelIndex& current, const QModelIndex& previous);
    // Select object by clicking on it
    void select_map_object(const std::string& object_name);
    // Select layer from list
    void select_layer(const QModelIndex& current, const QModelIndex& previous);
private:
    Ui::Main_Window* ui;
    Map_View* map_view;
    Property_Editor* prop_edit;
    QString project_dir;
    QString map_dir;
};

#endif // MAIN_WINDOW_HPP
