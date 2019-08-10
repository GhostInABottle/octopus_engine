#-------------------------------------------------
#
# Project created by QtCreator 2014-11-08T23:20:24
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = editor
TEMPLATE = app

exists($$PWD/machine_specific.txt) {
    include($$PWD/machine_specific.txt)
}

SOURCES += main.cpp\
    main_window.cpp \
    ../include/xd/vendor/glm/detail/glm.cpp \
    ../src/vendor/base64.cpp \
    ../src/camera.cpp \
    ../src/canvas.cpp \
    ../src/canvas_renderer.cpp \
    ../src/canvas_updater.cpp \
    ../src/clock.cpp \
    ../src/collision_record.cpp \
    ../src/command.cpp \
    ../src/command_result.cpp \
    ../src/commands/fade_music_command.cpp \
    ../src/commands/move_camera_command.cpp \
    ../src/commands/move_object_command.cpp \
    ../src/commands/move_object_to_command.cpp \
    ../src/commands/shake_screen_command.cpp \
    ../src/commands/show_pose_command.cpp \
    ../src/commands/show_text_command.cpp \
    ../src/commands/text_options.cpp \
    ../src/commands/tint_screen_command.cpp \
    ../src/commands/update_canvas_command.cpp \
    ../src/commands/update_layer_command.cpp \
    ../src/commands/wait_command.cpp \
    ../src/commands/zoom_command.cpp \
    ../src/configurations.cpp \
    ../src/custom_shaders.cpp \
    ../src/editable.cpp \
    ../src/Editable.cpp \
    ../src/game.cpp \
    ../src/image_layer.cpp \
    ../src/image_layer_renderer.cpp \
    ../src/image_layer_updater.cpp \
    ../src/layer.cpp \
    ../src/layer_renderer.cpp \
    ../src/log.cpp \
    ../src/map.cpp \
    ../src/map_object.cpp \
    ../src/object_layer.cpp \
    ../src/object_layer_renderer.cpp \
    ../src/object_layer_updater.cpp \
    ../src/pathfinder.cpp \
    ../src/player_controller.cpp \
    ../src/save_file.cpp \
    ../src/scripting_interface.cpp \
    ../src/shake_decorator.cpp \
    ../src/sprite.cpp \
    ../src/sprite_data.cpp \
    ../src/sprite_holder.cpp \
    ../src/text_parser.cpp \
    ../src/tile_layer.cpp \
    ../src/tile_layer_renderer.cpp \
    ../src/tileset.cpp \
    ../src/tmx_properties.cpp \
    ../src/utility.cpp \
    ../src/xd/audio/audio.cpp \
    ../src/xd/audio/music.cpp \
    ../src/xd/audio/sound.cpp \
    ../src/xd/graphics/font.cpp \
    ../src/xd/graphics/framebuffer.cpp \
    ../src/xd/graphics/image.cpp \
    ../src/xd/graphics/shader_program.cpp \
    ../src/xd/graphics/shaders.cpp \
    ../src/xd/graphics/simple_text_renderer.cpp \
    ../src/xd/graphics/sprite_batch.cpp \
    ../src/xd/graphics/stock_text_formatter.cpp \
    ../src/xd/graphics/text_formatter.cpp \
    ../src/xd/graphics/text_renderer.cpp \
    ../src/xd/graphics/texture.cpp \
    ../src/xd/lua/scheduler.cpp \
    ../src/xd/lua/virtual_machine.cpp \
    ../src/xd/system/input.cpp \
    ../src/xd/system/window.cpp \
    map_view.cpp \
    mappers/image_layer_mapper.cpp \
    mappers/layer_mapper.cpp \
    mappers/map_mapper.cpp \
    mappers/map_object_mapper.cpp \
    mappers/object_layer_mapper.cpp \
    property_editor.cpp \
    qtpropertybrowser/src/qtbuttonpropertybrowser.cpp \
    qtpropertybrowser/src/qteditorfactory.cpp \
    qtpropertybrowser/src/qtgroupboxpropertybrowser.cpp \
    qtpropertybrowser/src/qtpropertybrowser.cpp \
    qtpropertybrowser/src/qtpropertybrowserutils.cpp \
    qtpropertybrowser/src/qtpropertymanager.cpp \
    qtpropertybrowser/src/qttreepropertybrowser.cpp \
    qtpropertybrowser/src/qtvariantproperty.cpp

HEADERS  += main_window.hpp \
    ../include/vendor/base64.hpp \
    ../include/camera.hpp \
    ../include/canvas.hpp \
    ../include/canvas_renderer.hpp \
    ../include/canvas_updater.hpp \
    ../include/clock.hpp \
    ../include/collision_check_types.hpp \
    ../include/collision_record.hpp \
    ../include/command.hpp \
    ../include/command_result.hpp \
    ../include/commands.hpp \
    ../include/commands/fade_music_command.hpp \
    ../include/commands/move_camera_command.hpp \
    ../include/commands/move_object_command.hpp \
    ../include/commands/move_object_to_command.hpp \
    ../include/commands/shake_screen_command.hpp \
    ../include/commands/show_pose_command.hpp \
    ../include/commands/show_text_command.hpp \
    ../include/commands/text_options.hpp \
    ../include/commands/tint_screen_command.hpp \
    ../include/commands/update_canvas_command.hpp \
    ../include/commands/update_layer_command.hpp \
    ../include/commands/wait_command.hpp \
    ../include/commands/zoom_command.hpp \
    ../include/configurations.hpp \
    ../include/custom_shaders.hpp \
    ../include/direction.hpp \
    ../include/direction_utilities.hpp \
    ../include/editable.hpp \
    ../include/exceptions.hpp \
    ../include/game.hpp \
    ../include/image_layer.hpp \
    ../include/image_layer_renderer.hpp \
    ../include/image_layer_updater.hpp \
    ../include/layer.hpp \
    ../include/layer_renderer.hpp \
    ../include/layer_types.hpp \
    ../include/layer_updater.hpp \
    ../include/log.hpp \
    ../include/map.hpp \
    ../include/map_object.hpp \
    ../include/object_layer.hpp \
    ../include/object_layer_renderer.hpp \
    ../include/object_layer_updater.hpp \
    ../include/pathfinder.hpp \
    ../include/player_controller.hpp \
    ../include/vendor/rapidxml.hpp \
    ../include/vendor/rapidxml_print.hpp \
    ../include/save_file.hpp \
    ../include/scripting_interface.hpp \
    ../include/shake_decorator.hpp \
    ../include/sprite.hpp \
    ../include/sprite_data.hpp \
    ../include/sprite_holder.hpp \
    ../include/tests/game_fixture.hpp \
    ../include/text_parser.hpp \
    ../include/tile_layer.hpp \
    ../include/tile_layer_renderer.hpp \
    ../include/tileset.hpp \
    ../include/tmx_properties.hpp \
    ../include/utility.hpp \
    ../include/xd/asset_manager.hpp \
    ../include/xd/asset_serializer.hpp \
    ../include/xd/audio.hpp \
    ../include/xd/audio/audio.hpp \
    ../include/xd/audio/detail/audio_handle.hpp \
    ../include/xd/audio/exceptions.hpp \
    ../include/xd/audio/music.hpp \
    ../include/xd/audio/sound.hpp \
    ../include/xd/detail/entity.hpp \
    ../include/xd/detail/identity.hpp \
    ../include/xd/detail/noop_deleter.hpp \
    ../include/xd/entity.hpp \
    ../include/xd/event_bus.hpp \
    ../include/xd/exception.hpp \
    ../include/xd/glm.hpp \
    ../include/xd/graphics.hpp \
    ../include/xd/graphics/detail/font.hpp \
    ../include/xd/graphics/detail/image.hpp \
    ../include/xd/graphics/detail/sprite_batch.hpp \
    ../include/xd/graphics/detail/text_formatter.hpp \
    ../include/xd/graphics/detail/vertex_traits.hpp \
    ../include/xd/graphics/exceptions.hpp \
    ../include/xd/graphics/font.hpp \
    ../include/xd/graphics/font_style.hpp \
    ../include/xd/graphics/framebuffer.hpp \
    ../include/xd/graphics/image.hpp \
    ../include/xd/graphics/matrix_stack.hpp \
    ../include/xd/graphics/shader_program.hpp \
    ../include/xd/graphics/shaders.hpp \
    ../include/xd/graphics/simple_text_renderer.hpp \
    ../include/xd/graphics/sprite_batch.hpp \
    ../include/xd/graphics/stock_text_formatter.hpp \
    ../include/xd/graphics/text_formatter.hpp \
    ../include/xd/graphics/text_renderer.hpp \
    ../include/xd/graphics/texture.hpp \
    ../include/xd/graphics/transform_geometry.hpp \
    ../include/xd/graphics/types.hpp \
    ../include/xd/graphics/utility.hpp \
    ../include/xd/graphics/vertex_batch.hpp \
    ../include/xd/graphics/vertex_traits.hpp \
    ../include/xd/lua.hpp \
    ../include/xd/lua/detail/scheduler.hpp \
    ../include/xd/lua/detail/scheduler_task.hpp \
    ../include/xd/lua/exceptions.hpp \
    ../include/xd/lua/function.hpp \
    ../include/xd/lua/scheduler.hpp \
    ../include/xd/lua/scheduler_task.hpp \
    ../include/xd/lua/types.hpp \
    ../include/xd/lua/virtual_machine.hpp \
    ../include/xd/system.hpp \
    ../include/xd/system/exceptions.hpp \
    ../include/xd/system/input.hpp \
    ../include/xd/system/window.hpp \
    ../include/xd/system/window_options.hpp \
    ../include/xd/types.hpp \
    map_view.hpp \
    mappers/image_layer_mapper.hpp \
    mappers/layer_mapper.hpp \
    mappers/map_mapper.hpp \
    mappers/map_object_mapper.hpp \
    mappers/object_layer_mapper.hpp \
    property_editor.hpp \
    qtpropertybrowser/src/qtbuttonpropertybrowser.h \
    qtpropertybrowser/src/qteditorfactory.h \
    qtpropertybrowser/src/qtgroupboxpropertybrowser.h \
    qtpropertybrowser/src/qtpropertybrowser.h \
    qtpropertybrowser/src/qtpropertybrowserutils_p.h \
    qtpropertybrowser/src/qtpropertymanager.h \
    qtpropertybrowser/src/qttreepropertybrowser.h \
    qtpropertybrowser/src/qtvariantproperty.h \
    ui_main_window.h

FORMS    += main_window.ui

DISTFILES += \
    ../include/xd/vendor/glm/CMakeLists.txt
