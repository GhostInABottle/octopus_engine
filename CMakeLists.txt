cmake_minimum_required(VERSION 3.10)
project(octopus_engine)

option(STEAM_BUILD "Link with the Steam SDK?")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

file(GLOB SOURCES LIST_DIRECTORIES false src/*.cpp)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/test_main.cpp)

file(GLOB EXTRA_SOURCES LIST_DIRECTORIES false
    src/commands/*.cpp
    src/canvas/*.cpp
    src/decorators/*.cpp
    src/filesystem/*.cpp
    src/interfaces/*.cpp
    src/layers/*.cpp
    src/environments/*.cpp
    src/scripting/*.cpp
    src/scripting/bindings/*.cpp
    src/utility/*.cpp
    src/vendor/*.cpp)

file(GLOB XD_SOURCES LIST_DIRECTORIES false
    src/xd/audio/*.cpp
    src/xd/audio/detail/*.cpp
    src/xd/graphics/*.cpp
    src/xd/lua/*.cpp
    src/xd/system/*.cpp)

add_executable(octopus_engine ${SOURCES} ${EXTRA_SOURCES} ${XD_SOURCES})

target_compile_definitions(octopus_engine PRIVATE NDEBUG)
target_compile_features(octopus_engine PRIVATE cxx_std_17)

set(OpenGL_GL_PREFERENCE LEGACY)
set(IS_GNU "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

if (APPLE OR (${IS_GNU} AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8))
    find_package(Boost REQUIRED COMPONENTS filesystem)
else()
    find_package(Boost REQUIRED)
endif()
find_package(Freetype REQUIRED)
find_package(HarfBuzz REQUIRED)
find_package(Lua 5.2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(ZLIB REQUIRED)
find_package(FMOD REQUIRED)
find_package(PhysFS REQUIRED)
find_package(Steam_Api)

set(INCLUDE_DIRS ${LUA_INCLUDE_DIR} ${Boost_INCLUDE_DIRS}
${FREETYPE_INCLUDE_DIR_ft2build} ${PHYSFS_INCLUDE_DIR})

set(DEPENDENCIES ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${FREETYPE_LIBRARIES}
${HARFBUZZ_LIBRARIES} ${LUA_LIBRARIES} ${ZLIB_LIBRARIES} ${FMOD_LIBRARIES}
${HarfBuzz_LIBRARIES} ${PHYSFS_LIBRARY} GLEW::GLEW glfw)

set(DEFINITIONS "")

if (STEAM_BUILD AND STEAM_API_FOUND)
    list(APPEND INCLUDE_DIRS ${STEAM_API_INCLUDE_DIRS})
    list(APPEND DEPENDENCIES ${STEAM_API_LIBRARIES})
    list(APPEND DEFINITIONS OCB_USE_STEAM_SDK)
endif()

if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    list(APPEND DEPENDENCIES ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${Boost_LIBRARIES})

    list(APPEND DEFINITIONS OCB_USE_BOOST_FILESYSTEM OCB_USE_C_LUA)
    string(APPEND CMAKE_EXE_LINKER_FLAGS "-headerpad_max_install_names")
elseif(UNIX AND ${IS_GNU})
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
        message(FATAL_ERROR "GCC version 7 or later is required. Found:  " ${CMAKE_CXX_COMPILER_VERSION})
    elseif (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
        # Version 7 doesn't have <fileystem>, use boost's
        list(APPEND DEPENDENCIES ${Boost_LIBRARIES})
        list(APPEND DEFINITIONS OCB_USE_BOOST_FILESYSTEM)
    else(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        # Version 8 requires linking to stdc++fs
        list(APPEND DEPENDENCIES stdc++fs)
    endif()
endif()

target_compile_definitions(octopus_engine PRIVATE ${DEFINITIONS})
target_include_directories(octopus_engine PUBLIC ${INCLUDE_DIRS})
target_link_libraries(octopus_engine ${DEPENDENCIES})
