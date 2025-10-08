# Minimum CMake version (vcpkg recommends at least 3.16+)
cmake_minimum_required(VERSION 3.16)
project(QEngine)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies via vcpkg
find_package(imgui CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)


find_package(Lua REQUIRED)

# Source files
add_executable(QEngine
        src/main.cpp
        src/TextureLoader.cpp
        include/TextureLoader.h
        src/UI.cpp
        include/UI.h
        include/Sprite.h
        src/LuaScripting.cpp
        include/LuaScripting.h
        src/CodeEditor.cpp
        include/CodeEditor.h
        src/TextEditor.cpp
        include/TextEditor.h
        src/TextEditor.cpp
        src/AssetManager.cpp
        include/AssetManager.h
        src/Shader.cpp
        include/Shader.h
        include/Collision.h
        src/Collision.cpp
        src/animation.cpp
        src/animation.h
)

# Link against libraries
target_link_libraries(QEngine PRIVATE
        imgui::imgui
        SDL3::SDL3
        SDL3_image::SDL3_image
        glfw
        OpenGL::GL
        GLEW::GLEW


        ${LUA_LIBRARIES}



        glm::glm
)

# Correctly link Lua's include directories
target_include_directories(QEngine PRIVATE ${LUA_INCLUDE_DIR})

option(GAME "Toggles Editor" OFF)
if(GAME)
    target_compile_definitions(QEngine PUBLIC GAME_MODE=1)
else()
    target_compile_definitions(QEngine PUBLIC GAME_MODE=0)
endif()