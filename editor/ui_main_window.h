/********************************************************************************
** Form generated from reading UI file 'main_window.ui'
**
** Created by: Qt User Interface Compiler version 5.3.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAIN_WINDOW_H
#define UI_MAIN_WINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Main_Window
{
public:
    QAction *action_open_project;
    QAction *action_save;
    QAction *action_save_as;
    QAction *action_open_map;
    QAction *action_new_map;
    QWidget *central_widget;
    QHBoxLayout *widget_layout;
    QHBoxLayout *horizontal_layout;
    QScrollArea *scroll_area;
    QWidget *scroll_contents;
    QMenuBar *menu_bar;
    QMenu *menu_file;
    QToolBar *main_toolbar;
    QStatusBar *status_bar;
    QDockWidget *dock_widget;
    QWidget *dock_contents;
    QHBoxLayout *dock_layout;
    QVBoxLayout *lists_layout;
    QTabWidget *tab_widget;
    QWidget *map_tab;
    QGridLayout *gridLayout;
    QListView *map_list;
    QWidget *layer_tab;
    QGridLayout *gridLayout_2;
    QListView *layer_list;
    QWidget *object_tab;
    QGridLayout *gridLayout_3;
    QListView *object_list;

    void setupUi(QMainWindow *Main_Window)
    {
        if (Main_Window->objectName().isEmpty())
            Main_Window->setObjectName(QStringLiteral("Main_Window"));
        Main_Window->resize(821, 600);
        action_open_project = new QAction(Main_Window);
        action_open_project->setObjectName(QStringLiteral("action_open_project"));
        action_save = new QAction(Main_Window);
        action_save->setObjectName(QStringLiteral("action_save"));
        action_save_as = new QAction(Main_Window);
        action_save_as->setObjectName(QStringLiteral("action_save_as"));
        action_open_map = new QAction(Main_Window);
        action_open_map->setObjectName(QStringLiteral("action_open_map"));
        action_new_map = new QAction(Main_Window);
        action_new_map->setObjectName(QStringLiteral("action_new_map"));
        central_widget = new QWidget(Main_Window);
        central_widget->setObjectName(QStringLiteral("central_widget"));
        central_widget->setAutoFillBackground(true);
        widget_layout = new QHBoxLayout(central_widget);
        widget_layout->setSpacing(6);
        widget_layout->setContentsMargins(11, 11, 11, 11);
        widget_layout->setObjectName(QStringLiteral("widget_layout"));
        horizontal_layout = new QHBoxLayout();
        horizontal_layout->setSpacing(6);
        horizontal_layout->setObjectName(QStringLiteral("horizontal_layout"));
        scroll_area = new QScrollArea(central_widget);
        scroll_area->setObjectName(QStringLiteral("scroll_area"));
        scroll_area->setContextMenuPolicy(Qt::ActionsContextMenu);
        scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scroll_area->setWidgetResizable(true);
        scroll_contents = new QWidget();
        scroll_contents->setObjectName(QStringLiteral("scroll_contents"));
        scroll_contents->setGeometry(QRect(0, 0, 598, 525));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(scroll_contents->sizePolicy().hasHeightForWidth());
        scroll_contents->setSizePolicy(sizePolicy);
        scroll_area->setWidget(scroll_contents);

        horizontal_layout->addWidget(scroll_area);


        widget_layout->addLayout(horizontal_layout);

        Main_Window->setCentralWidget(central_widget);
        menu_bar = new QMenuBar(Main_Window);
        menu_bar->setObjectName(QStringLiteral("menu_bar"));
        menu_bar->setGeometry(QRect(0, 0, 821, 21));
        menu_file = new QMenu(menu_bar);
        menu_file->setObjectName(QStringLiteral("menu_file"));
        Main_Window->setMenuBar(menu_bar);
        main_toolbar = new QToolBar(Main_Window);
        main_toolbar->setObjectName(QStringLiteral("main_toolbar"));
        Main_Window->addToolBar(Qt::TopToolBarArea, main_toolbar);
        status_bar = new QStatusBar(Main_Window);
        status_bar->setObjectName(QStringLiteral("status_bar"));
        Main_Window->setStatusBar(status_bar);
        dock_widget = new QDockWidget(Main_Window);
        dock_widget->setObjectName(QStringLiteral("dock_widget"));
        sizePolicy.setHeightForWidth(dock_widget->sizePolicy().hasHeightForWidth());
        dock_widget->setSizePolicy(sizePolicy);
        dock_widget->setMinimumSize(QSize(200, 243));
        dock_widget->setMaximumSize(QSize(200, 524287));
        dock_widget->setAutoFillBackground(true);
        dock_widget->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
        dock_contents = new QWidget();
        dock_contents->setObjectName(QStringLiteral("dock_contents"));
        sizePolicy.setHeightForWidth(dock_contents->sizePolicy().hasHeightForWidth());
        dock_contents->setSizePolicy(sizePolicy);
        dock_contents->setMinimumSize(QSize(175, 0));
        dock_contents->setAutoFillBackground(true);
        dock_layout = new QHBoxLayout(dock_contents);
        dock_layout->setSpacing(6);
        dock_layout->setContentsMargins(11, 11, 11, 11);
        dock_layout->setObjectName(QStringLiteral("dock_layout"));
        lists_layout = new QVBoxLayout();
        lists_layout->setSpacing(6);
        lists_layout->setObjectName(QStringLiteral("lists_layout"));
        tab_widget = new QTabWidget(dock_contents);
        tab_widget->setObjectName(QStringLiteral("tab_widget"));
        map_tab = new QWidget();
        map_tab->setObjectName(QStringLiteral("map_tab"));
        gridLayout = new QGridLayout(map_tab);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        map_list = new QListView(map_tab);
        map_list->setObjectName(QStringLiteral("map_list"));
        map_list->setAutoFillBackground(true);
        map_list->setEditTriggers(QAbstractItemView::NoEditTriggers);

        gridLayout->addWidget(map_list, 0, 0, 1, 1);

        tab_widget->addTab(map_tab, QString());
        layer_tab = new QWidget();
        layer_tab->setObjectName(QStringLiteral("layer_tab"));
        gridLayout_2 = new QGridLayout(layer_tab);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        layer_list = new QListView(layer_tab);
        layer_list->setObjectName(QStringLiteral("layer_list"));
        layer_list->setAutoFillBackground(true);

        gridLayout_2->addWidget(layer_list, 0, 0, 1, 1);

        tab_widget->addTab(layer_tab, QString());
        object_tab = new QWidget();
        object_tab->setObjectName(QStringLiteral("object_tab"));
        gridLayout_3 = new QGridLayout(object_tab);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        object_list = new QListView(object_tab);
        object_list->setObjectName(QStringLiteral("object_list"));
        object_list->setAutoFillBackground(true);

        gridLayout_3->addWidget(object_list, 0, 0, 1, 1);

        tab_widget->addTab(object_tab, QString());

        lists_layout->addWidget(tab_widget);


        dock_layout->addLayout(lists_layout);

        dock_widget->setWidget(dock_contents);
        Main_Window->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dock_widget);

        menu_bar->addAction(menu_file->menuAction());
        menu_file->addAction(action_new_map);
        menu_file->addAction(action_open_project);
        menu_file->addAction(action_open_map);
        menu_file->addAction(action_save);
        menu_file->addAction(action_save_as);

        retranslateUi(Main_Window);

        tab_widget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(Main_Window);
    } // setupUi

    void retranslateUi(QMainWindow *Main_Window)
    {
        Main_Window->setWindowTitle(QApplication::translate("Main_Window", "Octopus Editor", 0));
        action_open_project->setText(QApplication::translate("Main_Window", "Open &Project...", 0));
        action_open_project->setShortcut(QApplication::translate("Main_Window", "Ctrl+P", 0));
        action_save->setText(QApplication::translate("Main_Window", "&Save", 0));
        action_save->setShortcut(QApplication::translate("Main_Window", "Ctrl+S", 0));
        action_save_as->setText(QApplication::translate("Main_Window", "Save As...", 0));
        action_open_map->setText(QApplication::translate("Main_Window", "&Open Map...", 0));
        action_open_map->setShortcut(QApplication::translate("Main_Window", "Ctrl+O", 0));
        action_new_map->setText(QApplication::translate("Main_Window", "&New Map", 0));
        action_new_map->setShortcut(QApplication::translate("Main_Window", "Ctrl+N", 0));
        menu_file->setTitle(QApplication::translate("Main_Window", "&File", 0));
        tab_widget->setTabText(tab_widget->indexOf(map_tab), QApplication::translate("Main_Window", "Maps", 0));
        tab_widget->setTabText(tab_widget->indexOf(layer_tab), QApplication::translate("Main_Window", "Layers", 0));
        tab_widget->setTabText(tab_widget->indexOf(object_tab), QApplication::translate("Main_Window", "Objects", 0));
    } // retranslateUi

};

namespace Ui {
    class Main_Window: public Ui_Main_Window {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAIN_WINDOW_H
