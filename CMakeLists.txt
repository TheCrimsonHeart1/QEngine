# Minimum CMake version (vcpkg recommends at least 3.16+)
cmake_minimum_required(VERSION 3.16)
project(QEngine)

# Set up vcpkg toolchain for manifest mode.
# Recommended approach: create a vcpkg.json manifest file instead
# and let CMake handle the toolchain file automatically.
# For manual setup: set(CMAKE_TOOLCHAIN_FILE "/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Use C++20
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

# Use CMake's FindLua module for locating Lua
find_package(Lua REQUIRED)

# Source files
add_executable(QEngine
        main.cpp
        TextureLoader.cpp
        TextureLoader.h
        UI.cpp
        UI.h
        Sprite.h
        LuaScripting.cpp
        LuaScripting.h
        CodeEditor.cpp
        CodeEditor.h
        TextEditor.cpp
        TextEditor.h
        TextEditor.cpp
        AssetManager.cpp
        AssetManager.h
        Shader.cpp
        Shader.h
)

# Link against libraries
target_link_libraries(QEngine PRIVATE
        imgui::imgui
        SDL3::SDL3
        SDL3_image::SDL3_image
        glfw
        OpenGL::GL
        GLEW::GLEW

        # Correct way to link Lua using variables from find_package(Lua)
        ${LUA_LIBRARIES}

        # The include directories are also needed for the header files
        # The correct way to include is with target_include_directories\]

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