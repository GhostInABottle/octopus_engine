#include <QStandardItemModel>
#include <QStandardItem>
#include <QList>
#include <QMessageBox>
#include <QFileDialog>
#include <QDirIterator>
#include "main_window.hpp"
#include "ui_main_window.h"
#include "property_editor.hpp"

Main_Window::Main_Window(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::Main_Window),
    map_dir(".") {
    ui->setupUi(this);
    map_view = new Map_View(ui->scroll_area);
    ui->scroll_area->setWidget(map_view);
    ui->map_list->setModel(new QStandardItemModel(this));
    ui->layer_list->setModel(new QStandardItemModel(this));
    ui->object_list->setModel(new QStandardItemModel(this));
    connect(ui->action_new_map, &QAction::triggered,
            map_view, &Map_View::new_map);
    connect(ui->action_open_project, &QAction::triggered,
            this, &Main_Window::open_project);
    connect(ui->action_open_map, &QAction::triggered,
            map_view, &Map_View::open_map);
    connect(ui->action_save, &QAction::triggered,
            map_view, &Map_View::save);
    connect(ui->action_save_as, &QAction::triggered,
            map_view, &Map_View::save_as);
    connect(map_view, &Map_View::map_changed,
            this, &Main_Window::update_map);
    connect(map_view, &Map_View::object_dragged,
            this, &Main_Window::drag_object);
    connect(map_view, &Map_View::object_selected,
            this, &Main_Window::select_map_object);
    connect(ui->map_list, &QListView::activated,
            this, &Main_Window::select_map);
    connect(ui->object_list->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &Main_Window::select_list_object);
    connect(ui->layer_list->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &Main_Window::select_layer);

    prop_edit = new Property_Editor();
    ui->lists_layout->addWidget(prop_edit);
    prop_edit->show();
}

Main_Window::~Main_Window() {
    delete ui;
}

void Main_Window::open_project() {
    // Show file dialog
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    QStringList filenames;
    if (!dialog.exec())
        return;
    filenames = dialog.selectedFiles();
    // Check if map dir exists
    map_dir = filenames[0] + "/data/maps";
    if (!QDir(map_dir).exists())
        return;
    project_dir = filenames[0];
    QDir::setCurrent(project_dir);
    // Add map names to map list
    QDirIterator it(map_dir, QStringList() << "*.tmx", QDir::Files);
    auto map_model = static_cast<QStandardItemModel*>(ui->map_list->model());
    map_model->clear();
    while (it.hasNext()) {
        QString base_name = QFileInfo(it.next()).baseName();
        map_model->appendRow(new QStandardItem(base_name));
    }
}

void Main_Window::drag_object(QPoint new_position) {
    prop_edit->update_property("Position", new_position);
}

void Main_Window::select_map(const QModelIndex & index) {
    auto map_model = static_cast<QStandardItemModel*>(ui->map_list->model());
    QString base_name = map_model->itemFromIndex(index)->text();
    QString filename = map_dir + "/" + base_name + ".tmx";
    if (QFileInfo(filename).exists()) {
        map_view->load_map(filename.toStdString());
    }
    prop_edit->set_map(*map_view->get_game(), map_view->get_map());
}

void Main_Window::update_map(std::string filename) {
    auto layer_names = map_view->get_layer_names();
    auto layer_model = static_cast<QStandardItemModel*>(ui->layer_list->model());
    layer_model->clear();
    for (auto& layer : layer_names) {
        layer_model->appendRow(new QStandardItem(QString::fromStdString(layer)));
    }
    auto object_names = map_view->get_object_names();
    auto object_model = static_cast<QStandardItemModel*>(ui->object_list->model());
    object_model->clear();
    for (auto& obj : object_names) {
        object_model->appendRow(new QStandardItem(QString::fromStdString(obj)));
    }
    prop_edit->set_map(*map_view->get_game(), map_view->get_map());
    auto map_model = static_cast<QStandardItemModel*>(ui->map_list->model());
    if (!filename.empty()) {
        QString base_name = QFileInfo(QString::fromStdString(filename)).baseName();
        auto items = map_model->findItems(base_name);
        if (items.empty())
            map_model->appendRow(new QStandardItem(base_name));
        else
            ui->map_list->setCurrentIndex(items.first()->index());
    } else {
        map_model->appendRow(new QStandardItem("Untitled"));
    }
}

void Main_Window::select_list_object(const QModelIndex& current, const QModelIndex&) {
    auto object_model = static_cast<QStandardItemModel*>(ui->object_list->model());
    auto item = object_model->itemFromIndex(current);
    auto& game = *map_view->get_game();
    auto map_object = map_view->get_object(item->text().toStdString());
    prop_edit->set_map_object(game, map_object);
    map_view->highlight_object(map_object);
}

void Main_Window::select_map_object(const std::string& object_name) {
    auto& game = *map_view->get_game();
    if (object_name.empty()) {
        prop_edit->set_map_object(game, nullptr);
        return;
    }
    auto name = QString::fromStdString(object_name);
    auto object_model = static_cast<QStandardItemModel*>(ui->object_list->model());
    auto items = object_model->findItems(name);
    if (!items.empty())
        ui->object_list->setCurrentIndex(items.first()->index());
    auto map_object = map_view->get_object(object_name);
    prop_edit->set_map_object(game, map_object);
    map_view->highlight_object(map_object);
}

void Main_Window::select_layer(const QModelIndex& current, const QModelIndex&) {
    auto object_model = static_cast<QStandardItemModel*>(ui->layer_list->model());
    auto item = object_model->itemFromIndex(current);
    auto& game = *map_view->get_game();
    auto layer = map_view->get_layer(item->text().toStdString());
    prop_edit->set_layer(game, layer);
}
