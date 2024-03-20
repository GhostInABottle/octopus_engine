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
    ../src/vendor/lutf8lib.cpp \
    ../src/vendor/platform_folders.cpp \
    ../src/vendor/physfs.cpp \
    ../src/camera.cpp \
    ../src/canvas/base_canvas.cpp \
    ../src/canvas/base_image_canvas.cpp \
    ../src/canvas/canvas_renderer.cpp \
    ../src/canvas/canvas_updater.cpp \
    ../src/canvas/image_canvas.cpp \
    ../src/canvas/sprite_canvas.cpp \
    ../src/canvas/text_canvas.cpp \
    ../src/audio_player.cpp \
    ../src/clock.cpp \
    ../src/commands/command.cpp \
    ../src/commands/command_result.cpp \
    ../src/commands/timed_command.cpp \
    ../src/commands/fade_music_command.cpp \
    ../src/commands/move_camera_command.cpp \
    ../src/commands/move_canvas_command.cpp \
    ../src/commands/move_object_command.cpp \
    ../src/commands/move_object_to_command.cpp \
    ../src/commands/shake_screen_command.cpp \
    ../src/commands/show_pose_command.cpp \
    ../src/commands/show_text_command.cpp \
    ../src/commands/text_options.cpp \
    ../src/commands/tint_command.cpp \
    ../src/commands/update_image_command.cpp \
    ../src/commands/update_color_command.cpp \
    ../src/commands/update_opacity_command.cpp \
    ../src/commands/update_layer_velocity_command.cpp \
    ../src/commands/wait_command.cpp \
    ../src/commands/zoom_command.cpp \
    ../src/configurations.cpp \
    ../src/custom_shaders.cpp \
    ../src/interfaces/editable.cpp \
    ../src/interfaces/sprite_holder.cpp \
    ../src/game.cpp \
    ../src/layers/image_layer.cpp \
    ../src/layers/image_layer_renderer.cpp \
    ../src/layers/image_layer_updater.cpp \
    ../src/key_binder.cpp \
    ../src/layers/layer.cpp \
    ../src/layers/layer_renderer.cpp \
    ../src/log.cpp \
    ../src/map.cpp \
    ../src/map_object.cpp \
    ../src/layers/object_layer.cpp \
    ../src/layers/object_layer_renderer.cpp \
    ../src/layers/object_layer_updater.cpp \
    ../src/pathfinder.cpp \
    ../src/player_controller.cpp \
    ../src/save_file.cpp \
    ../src/scripting/bindings/canvas_bindings.cpp \
    ../src/scripting/bindings/environment_bindings.cpp \
    ../src/scripting/bindings/game_bindings.cpp \
    ../src/scripting/bindings/audio_bindings.cpp \
    ../src/scripting/bindings/camera_bindings.cpp \
    ../src/scripting/bindings/file_bindings.cpp \
    ../src/scripting/bindings/layer_bindings.cpp \
    ../src/scripting/bindings/map_bindings.cpp \
    ../src/scripting/bindings/map_object_bindings.cpp \
    ../src/scripting/bindings/math_bindings.cpp \
    ../src/scripting/bindings/text_bindings.cpp \
    ../src/scripting/bindings/utility_bindings.cpp \
    ../src/scripting/lua_object.cpp \
    ../src/scripting/scripting_interface.cpp \
    ../src/decorators/shake_decorator.cpp \
    ../src/decorators/typewriter_decorator.cpp \
    ../src/environments/default_environment.cpp \
    ../src/environments/steam_environment.cpp \
    ../src/sprite.cpp \
    ../src/sprite_data.cpp \
    ../src/text_parser.cpp \
    ../src/layers/tile_layer.cpp \
    ../src/layers/tile_layer_renderer.cpp \
    ../src/tileset.cpp \
    ../src/tmx_properties.cpp \
    ../src/utility/math.cpp \
    ../src/utility/file.cpp \
    ../src/utility/string.cpp \
    ../src/utility/color.cpp \
    ../src/utility/xml.cpp \
    ../src/filesystem/boost_filesystem.cpp \
    ../src/filesystem/disk_filesystem.cpp \
    ../src/filesystem/physfs_filesystem.cpp \
    ../src/filesystem/standard_filesystem.cpp \
    ../src/filesystem/user_data_folder.cpp \
    ../src/xd/audio/detail/fmod_audio_handle.cpp \
    ../src/xd/audio/detail/fmod_sound_handle.cpp \
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
    ../include/vendor/lutf8lib.hpp \
    ../include/vendor/platform_folders.hpp \
    ../include/vendor/rapidxml.hpp \
    ../include/vendor/rapidxml_print.hpp \
    ../include/vendor/unidata.h \
    ../include/vendor/utf8conv.h \
    ../include/vendor/physfs.hpp \
    ../include/camera.hpp \
    ../include/canvas/base_canvas.hpp \
    ../include/canvas/base_image_canvas.hpp \
    ../include/canvas/canvas_renderer.hpp \
    ../include/canvas/canvas_updater.hpp \
    ../include/canvas/image_canvas.hpp \
    ../include/canvas/sprite_canvas.hpp \
    ../include/canvas/text_canvas.hpp \
    ../include/audio_player.hpp \
    ../include/clock.hpp \
    ../include/collision_check_types.hpp \
    ../include/collision_record.hpp \
    ../include/commands/command.hpp \
    ../include/commands/command_result.hpp \
    ../include/commands/timed_command.hpp \
    ../include/commands/fade_music_command.hpp \
    ../include/commands/move_camera_command.hpp \
    ../include/commands/move_canvas_command.hpp \
    ../include/commands/move_object_command.hpp \
    ../include/commands/move_object_to_command.hpp \
    ../include/commands/shake_screen_command.hpp \
    ../include/commands/show_pose_command.hpp \
    ../include/commands/show_text_command.hpp \
    ../include/commands/text_options.hpp \
    ../include/commands/tint_command.hpp \
    ../include/commands/update_image_command.hpp \
    ../include/commands/update_color_command.hpp \
    ../include/commands/update_opacity_command.hpp \
    ../include/commands/update_layer_velocity_command.hpp \
    ../include/commands/wait_command.hpp \
    ../include/commands/zoom_command.hpp \
    ../include/configurations.hpp \
    ../include/custom_shaders.hpp \
    ../include/direction.hpp \
    ../include/exceptions.hpp \
    ../include/game.hpp \
    ../include/interfaces/editable.hpp \
    ../include/interfaces/opacity_holder.hpp \
    ../include/interfaces/sprite_holder.hpp \
    ../include/layers/image_layer.hpp \
    ../include/layers/image_layer_renderer.hpp \
    ../include/layers/image_layer_updater.hpp \
    ../include/key_binder.hpp \
    ../include/layers/layer.hpp \
    ../include/layers/layer_renderer.hpp \
    ../include/layers/layer_types.hpp \
    ../include/layers/layer_updater.hpp \
    ../include/log.hpp \
    ../include/map.hpp \
    ../include/map_object.hpp \
    ../include/layers/object_layer.hpp \
    ../include/layers/object_layer_renderer.hpp \
    ../include/layers/object_layer_updater.hpp \
    ../include/pathfinder.hpp \
    ../include/player_controller.hpp \
    ../include/vendor/rapidxml.hpp \
    ../include/vendor/rapidxml_print.hpp \
    ../include/save_file.hpp \
    ../include/scripting/script_bindings.hpp \
    ../include/scripting/lua_object.hpp \
    ../include/scripting/scripting_interface.hpp \
    ../include/decorators/shake_decorator.hpp \
    ../include/decorators/typewriter_decorator.hpp \
    ../include/environments/default_environment.hpp \
    ../include/environments/environment.hpp \
    ../include/environments/steam_environment.hpp \
    ../include/sprite.hpp \
    ../include/sprite_data.hpp \
    ../include/tests/game_fixture.hpp \
    ../include/text_parser.hpp \
    ../include/layers/tile_layer.hpp \
    ../include/layers/tile_layer_renderer.hpp \
    ../include/tileset.hpp \
    ../include/tmx_properties.hpp \
    ../include/utility/color.hpp \
    ../include/utility/direction.hpp \
    ../include/utility/file.hpp \
    ../include/utility/math.hpp \
    ../include/utility/string.hpp \
    ../include/utility/xml.hpp \
    ../include/filesystem/boost_filesystem.hpp \
    ../include/filesystem/disk_filesystem.hpp \
    ../include/filesystem/physfs_filesystem.hpp \
    ../include/filesystem/path_info.hpp \
    ../include/filesystem/readable_filesystem.hpp \
    ../include/filesystem/standard_filesystem.hpp \
    ../include/filesystem/user_data_folder.hpp \
    ../include/filesystem/writable_filesystem.hpp \
    ../include/xd/asset_manager.hpp \
    ../include/xd/asset_serializer.hpp \
    ../include/xd/audio.hpp \
    ../include/xd/audio/audio.hpp \
    ../include/xd/audio/detail/audio_handle.hpp \
    ../include/xd/audio/detail/sound_handle.hpp \
    ../include/xd/audio/detail/fmod_audio_handle.hpp \
    ../include/xd/audio/detail/fmod_sound_handle.hpp \
    ../include/xd/audio/channel_group_type.hpp \
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
    ../include/xd/lua/exceptions.hpp \
    ../include/xd/lua/scheduler.hpp \
    ../include/xd/lua/scheduler_task.hpp \
    ../include/xd/lua/virtual_machine.hpp \
    ../include/xd/system/exceptions.hpp \
    ../include/xd/system/input.hpp \
    ../include/xd/system/window.hpp \
    ../include/xd/system/window_options.hpp \
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
    qtpropertybrowser/src/qtvariantproperty.h

FORMS    += main_window.ui

DISTFILES += \
    ../include/xd/vendor/glm/CMakeLists.txt

QMAKE_CXXFLAGS += /std:c++17 /bigobj
