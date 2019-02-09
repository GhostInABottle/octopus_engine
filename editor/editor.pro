#-------------------------------------------------
#
# Project created by QtCreator 2014-11-08T23:20:24
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = editor
TEMPLATE = app

INCLUDEPATH += $$PWD/../vendor/xd/include

exists($$PWD/machine_specific.txt) {
    include($$PWD/machine_specific.txt)
}

SOURCES += main.cpp\
        main_window.cpp \
    map_view.cpp \
    ../src/base64.cpp \
    ../src/camera.cpp \
    ../src/canvas.cpp \
    ../src/canvas_renderer.cpp \
    ../src/canvas_updater.cpp \
    ../src/clock.cpp \
    ../src/collision_record.cpp \
    ../src/command_result.cpp \
    ../src/configurations.cpp \
    ../src/custom_shaders.cpp \
    ../src/game.cpp \
    ../src/image_layer.cpp \
    ../src/image_layer_renderer.cpp \
    ../src/image_layer_updater.cpp \
    ../src/layer.cpp \
    ../src/log.cpp \
    ../src/map.cpp \
    ../src/map_object.cpp \
    ../src/object_layer.cpp \
    ../src/object_layer_renderer.cpp \
    ../src/object_layer_updater.cpp \
    ../src/pathfinder.cpp \
    ../src/player_controller.cpp \
    ../src/scripting_interface.cpp \
    ../src/sprite.cpp \
    ../src/sprite_data.cpp \
    ../src/tile_layer.cpp \
    ../src/tile_layer_renderer.cpp \
    ../src/tileset.cpp \
    ../src/utility.cpp \
    ../src/save_file.cpp \
    ../vendor/xd/src/audio/audio.cpp \
    ../vendor/xd/src/audio/music.cpp \
    ../vendor/xd/src/audio/sound.cpp \
    ../vendor/xd/src/graphics/font.cpp \
    ../vendor/xd/src/graphics/image.cpp \
    ../vendor/xd/src/graphics/shader_program.cpp \
    ../vendor/xd/src/graphics/shaders.cpp \
    ../vendor/xd/src/graphics/simple_text_renderer.cpp \
    ../vendor/xd/src/graphics/sprite_batch.cpp \
    ../vendor/xd/src/graphics/stock_text_formatter.cpp \
    ../vendor/xd/src/graphics/text_formatter.cpp \
    ../vendor/xd/src/graphics/text_renderer.cpp \
    ../vendor/xd/src/graphics/texture.cpp \
    ../vendor/xd/src/lua/scheduler.cpp \
    ../vendor/xd/src/lua/virtual_machine.cpp \
    ../vendor/xd/src/system/input.cpp \
    ../vendor/xd/src/system/window.cpp \
    qtpropertybrowser/src/qtbuttonpropertybrowser.cpp \
    qtpropertybrowser/src/qteditorfactory.cpp \
    qtpropertybrowser/src/qtgroupboxpropertybrowser.cpp \
    qtpropertybrowser/src/qtpropertybrowser.cpp \
    qtpropertybrowser/src/qtpropertybrowserutils.cpp \
    qtpropertybrowser/src/qtpropertymanager.cpp \
    qtpropertybrowser/src/qttreepropertybrowser.cpp \
    qtpropertybrowser/src/qtvariantproperty.cpp \
    property_editor.cpp \
    mappers/map_object_mapper.cpp \
    ../src/sprite_holder.cpp \
    mappers/image_layer_mapper.cpp \
    mappers/layer_mapper.cpp \
    mappers/object_layer_mapper.cpp \
    mappers/map_mapper.cpp \
    ../src/layer_renderer.cpp \
    ../vendor/xd/include/xd/vendor/glm/detail/glm.cpp \
    ../src/command.cpp \
    ../src/commands/fade_music_command.cpp \
    ../src/commands/move_camera_command.cpp \
    ../src/commands/move_object_command.cpp \
    ../src/commands/move_object_to_command.cpp \
    ../src/commands/shake_screen_command.cpp \
    ../src/commands/show_pose_command.cpp \
    ../src/commands/show_text_command.cpp \
    ../src/commands/tint_screen_command.cpp \
    ../src/commands/update_canvas_command.cpp \
    ../src/commands/update_layer_command.cpp \
    ../src/commands/wait_command.cpp \
    ../vendor/xd/src/graphics/framebuffer.cpp \
    ../src/text_parser.cpp
    

HEADERS  += main_window.hpp \
    map_view.hpp \
    ../include/base64.hpp \
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
    ../include/common.hpp \
    ../include/configurations.hpp \
    ../include/custom_shaders.hpp \
    ../include/direction.hpp \
    ../include/direction_utilities.hpp \
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
    ../include/rapidxml.hpp \
    ../include/scripting_interface.hpp \
    ../include/sprite.hpp \
    ../include/sprite_data.hpp \
    ../include/sprite_holder.hpp \
    ../include/tile_layer.hpp \
    ../include/tile_layer_renderer.hpp \
    ../include/tileset.hpp \
    ../include/utility.hpp \
    ../include/save_file.hpp \
    ../vendor/xd/include/xd/audio/detail/audio_handle.hpp \
    ../vendor/xd/include/xd/audio/audio.hpp \
    ../vendor/xd/include/xd/audio/exceptions.hpp \
    ../vendor/xd/include/xd/audio/music.hpp \
    ../vendor/xd/include/xd/audio/sound.hpp \
    ../vendor/xd/include/xd/detail/entity.hpp \
    ../vendor/xd/include/xd/detail/identity.hpp \
    ../vendor/xd/include/xd/detail/iterate_asset_manager.hpp \
    ../vendor/xd/include/xd/detail/iterate_create.hpp \
    ../vendor/xd/include/xd/detail/iterate_entity_constructor.hpp \
    ../vendor/xd/include/xd/detail/iterate_factory_create.hpp \
    ../vendor/xd/include/xd/detail/noop_deleter.hpp \
    ../vendor/xd/include/xd/graphics/detail/font.hpp \
    ../vendor/xd/include/xd/graphics/detail/image.hpp \
    ../vendor/xd/include/xd/graphics/detail/iterate_render.hpp \
    ../vendor/xd/include/xd/graphics/detail/sprite_batch.hpp \
    ../vendor/xd/include/xd/graphics/detail/text_formatter.hpp \
    ../vendor/xd/include/xd/graphics/detail/vertex_traits.hpp \
    ../vendor/xd/include/xd/graphics/exceptions.hpp \
    ../vendor/xd/include/xd/graphics/font.hpp \
    ../vendor/xd/include/xd/graphics/font_style.hpp \
    ../vendor/xd/include/xd/graphics/image.hpp \
    ../vendor/xd/include/xd/graphics/matrix_stack.hpp \
    ../vendor/xd/include/xd/graphics/shader_program.hpp \
    ../vendor/xd/include/xd/graphics/shaders.hpp \
    ../vendor/xd/include/xd/graphics/simple_text_renderer.hpp \
    ../vendor/xd/include/xd/graphics/sprite_batch.hpp \
    ../vendor/xd/include/xd/graphics/stock_text_formatter.hpp \
    ../vendor/xd/include/xd/graphics/text_formatter.hpp \
    ../vendor/xd/include/xd/graphics/text_renderer.hpp \
    ../vendor/xd/include/xd/graphics/texture.hpp \
    ../vendor/xd/include/xd/graphics/transform_geometry.hpp \
    ../vendor/xd/include/xd/graphics/types.hpp \
    ../vendor/xd/include/xd/graphics/utility.hpp \
    ../vendor/xd/include/xd/graphics/vertex_batch.hpp \
    ../vendor/xd/include/xd/graphics/vertex_traits.hpp \
    ../vendor/xd/include/xd/lua/detail/iterate_function_call.hpp \
    ../vendor/xd/include/xd/lua/detail/iterate_scheduler_yield.hpp \
    ../vendor/xd/include/xd/lua/detail/scheduler.hpp \
    ../vendor/xd/include/xd/lua/detail/scheduler_task.hpp \
    ../vendor/xd/include/xd/lua/config.hpp \
    ../vendor/xd/include/xd/lua/exceptions.hpp \
    ../vendor/xd/include/xd/lua/function.hpp \
    ../vendor/xd/include/xd/lua/scheduler.hpp \
    ../vendor/xd/include/xd/lua/scheduler_task.hpp \
    ../vendor/xd/include/xd/lua/types.hpp \
    ../vendor/xd/include/xd/lua/virtual_machine.hpp \
    ../vendor/xd/include/xd/system/exceptions.hpp \
    ../vendor/xd/include/xd/system/input.hpp \
    ../vendor/xd/include/xd/system/utility.hpp \
    ../vendor/xd/include/xd/system/window.hpp \
    ../vendor/xd/include/xd/system/window_options.hpp \
    ../vendor/xd/include/xd/vendor/glew/glew.h \
    ../vendor/xd/include/xd/vendor/glew/glxew.h \
    ../vendor/xd/include/xd/vendor/glew/wglew.h \
    ../vendor/xd/include/xd/vendor/utf8/checked.h \
    ../vendor/xd/include/xd/vendor/utf8/core.h \
    ../vendor/xd/include/xd/vendor/utf8/unchecked.h \
    ../vendor/xd/include/xd/vendor/utf8.h \
    ../vendor/xd/include/xd/asset_manager.hpp \
    ../vendor/xd/include/xd/asset_serializer.hpp \
    ../vendor/xd/include/xd/audio.hpp \
    ../vendor/xd/include/xd/config.hpp \
    ../vendor/xd/include/xd/entity.hpp \
    ../vendor/xd/include/xd/event_bus.hpp \
    ../vendor/xd/include/xd/exception.hpp \
    ../vendor/xd/include/xd/factory.hpp \
    ../vendor/xd/include/xd/glm.hpp \
    ../vendor/xd/include/xd/graphics.hpp \
    ../vendor/xd/include/xd/handle.hpp \
    ../vendor/xd/include/xd/lua.hpp \
    ../vendor/xd/include/xd/ref_counted.hpp \
    ../vendor/xd/include/xd/system.hpp \
    ../vendor/xd/include/xd/types.hpp \
    ../vendor/xd/include/xd/weak_handle.hpp \
    qtpropertybrowser/src/qtbuttonpropertybrowser.h \
    qtpropertybrowser/src/qteditorfactory.h \
    qtpropertybrowser/src/qtgroupboxpropertybrowser.h \
    qtpropertybrowser/src/qtpropertybrowser.h \
    qtpropertybrowser/src/qtpropertybrowserutils_p.h \
    qtpropertybrowser/src/qtpropertymanager.h \
    qtpropertybrowser/src/qttreepropertybrowser.h \
    qtpropertybrowser/src/qtvariantproperty.h \
    ui_main_window.h \
    property_editor.hpp \
    ../include/editable.hpp \
    mappers/map_object_mapper.hpp \
    mappers/image_layer_mapper.hpp \
    mappers/layer_mapper.hpp \
    mappers/object_layer_mapper.hpp \
    mappers/map_mapper.hpp \
    ../include/rapidxml_print.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/_features.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/_fixes.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/_noise.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/_swizzle.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/_swizzle_func.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/_vectorize.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/func_common.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/func_exponential.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/func_geometric.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/func_integer.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/func_matrix.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/func_packing.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/func_trigonometric.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/func_vector_relational.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/intrinsic_common.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/intrinsic_exponential.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/intrinsic_geometric.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/intrinsic_integer.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/intrinsic_matrix.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/intrinsic_trigonometric.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/intrinsic_vector_relational.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/precision.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/setup.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_float.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_gentype.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_half.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_int.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_mat.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_mat2x2.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_mat2x3.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_mat2x4.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_mat3x2.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_mat3x3.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_mat3x4.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_mat4x2.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_mat4x3.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_mat4x4.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_vec.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_vec1.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_vec2.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_vec3.hpp \
    ../vendor/xd/include/xd/vendor/glm/detail/type_vec4.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/bitfield.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/color_space.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/constants.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/epsilon.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/integer.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/matrix_access.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/matrix_integer.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/matrix_inverse.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/matrix_transform.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/noise.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/packing.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/quaternion.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/random.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/reciprocal.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/round.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/type_precision.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/type_ptr.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/ulp.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtc/vec1.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/associated_min_max.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/bit.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/closest_point.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/color_space.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/color_space_YCoCg.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/common.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/compatibility.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/component_wise.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/dual_quaternion.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/euler_angles.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/extend.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/extented_min_max.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/fast_exponential.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/fast_square_root.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/fast_trigonometry.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/gradient_paint.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/handed_coordinate_space.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/hash.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/integer.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/intersect.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/io.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/log_base.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/matrix_cross_product.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/matrix_decompose.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/matrix_interpolation.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/matrix_major_storage.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/matrix_operation.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/matrix_query.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/matrix_transform_2d.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/mixed_product.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/norm.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/normal.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/normalize_dot.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/number_precision.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/optimum_pow.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/orthonormalize.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/perpendicular.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/polar_coordinates.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/projection.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/quaternion.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/range.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/raw_data.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/rotate_normalized_axis.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/rotate_vector.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/scalar_multiplication.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/scalar_relational.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/simd_mat4.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/simd_quat.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/simd_vec4.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/spline.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/std_based_type.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/string_cast.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/transform.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/transform2.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/type_aligned.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/vector_angle.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/vector_query.hpp \
    ../vendor/xd/include/xd/vendor/glm/gtx/wrap.hpp \
    ../vendor/xd/include/xd/vendor/glm/common.hpp \
    ../vendor/xd/include/xd/vendor/glm/exponential.hpp \
    ../vendor/xd/include/xd/vendor/glm/ext.hpp \
    ../vendor/xd/include/xd/vendor/glm/fwd.hpp \
    ../vendor/xd/include/xd/vendor/glm/geometric.hpp \
    ../vendor/xd/include/xd/vendor/glm/glm.hpp \
    ../vendor/xd/include/xd/vendor/glm/integer.hpp \
    ../vendor/xd/include/xd/vendor/glm/mat2x2.hpp \
    ../vendor/xd/include/xd/vendor/glm/mat2x3.hpp \
    ../vendor/xd/include/xd/vendor/glm/mat2x4.hpp \
    ../vendor/xd/include/xd/vendor/glm/mat3x2.hpp \
    ../vendor/xd/include/xd/vendor/glm/mat3x3.hpp \
    ../vendor/xd/include/xd/vendor/glm/mat3x4.hpp \
    ../vendor/xd/include/xd/vendor/glm/mat4x2.hpp \
    ../vendor/xd/include/xd/vendor/glm/mat4x3.hpp \
    ../vendor/xd/include/xd/vendor/glm/mat4x4.hpp \
    ../vendor/xd/include/xd/vendor/glm/matrix.hpp \
    ../vendor/xd/include/xd/vendor/glm/packing.hpp \
    ../vendor/xd/include/xd/vendor/glm/trigonometric.hpp \
    ../vendor/xd/include/xd/vendor/glm/vec2.hpp \
    ../vendor/xd/include/xd/vendor/glm/vec3.hpp \
    ../vendor/xd/include/xd/vendor/glm/vec4.hpp \
    ../vendor/xd/include/xd/vendor/glm/vector_relational.hpp \
    ../include/commands/fade_music_command.hpp \
    ../include/commands/move_camera_command.hpp \
    ../include/commands/move_object_command.hpp \
    ../include/commands/move_object_to_command.hpp \
    ../include/commands/shake_screen_command.hpp \
    ../include/commands/show_pose_command.hpp \
    ../include/commands/show_text_command.hpp \
    ../include/commands/tint_screen_command.hpp \
    ../include/commands/update_canvas_command.hpp \
    ../include/commands/update_layer_command.hpp \
    ../include/commands/wait_command.hpp \
    ../vendor/xd/include/xd/graphics/framebuffer.hpp \
    ../include/text_parser.hpp

FORMS    += main_window.ui
