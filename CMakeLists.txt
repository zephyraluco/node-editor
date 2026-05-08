cmake_minimum_required(VERSION 3.20)
project(node_editor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build SDL from the bundled source tree only (no external package manager).
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS        OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL      OFF CACHE BOOL "" FORCE)
# set static/shared options to ensure we get a static library (and avoid the need to copy DLLs on Windows)
set(SDL_SHARED       OFF CACHE BOOL "" FORCE)
set(SDL_STATIC       ON  CACHE BOOL "" FORCE)
add_subdirectory(libs/SDL EXCLUDE_FROM_ALL)

set(IMGUI_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui)
set(IMNODES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/imnodes)

add_executable(node_editor
    src/main.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMNODES_DIR}/imnodes.cpp
)

target_include_directories(node_editor PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMNODES_DIR}
)

# SDL3::SDL3 resolves to shared (or static) target SDL provides
target_link_libraries(node_editor PRIVATE SDL3::SDL3-static)

if(WIN32)
    target_link_libraries(node_editor PRIVATE opengl32)
    # Copy SDL3.dll next to the .exe after each build
    # add_custom_command(TARGET node_editor POST_BUILD
    #     COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #         $<TARGET_FILE:SDL3::SDL3>
    #         $<TARGET_FILE_DIR:node_editor>
    #     COMMENT "Copying SDL3.dll to output directory"
    # )
elseif(APPLE)
    find_library(OPENGL_FRAMEWORK OpenGL REQUIRED)
    target_link_libraries(node_editor PRIVATE ${OPENGL_FRAMEWORK})
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(node_editor PRIVATE OpenGL::GL)
endif()
