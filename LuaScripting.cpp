extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include "TextureLoader.h"
#include <vector>
#include "Sprite.h"
#include <GLFW/glfw3.h>
#include "LuaScripting.h"
#include <fstream>
#include <iostream>
#include <filesystem>

// The sprite vector from your engine (accessible to Lua)
extern std::vector<Sprite> sprites;

// Optional: global project folder system
extern std::filesystem::path assetFolder;
extern std::string AssetPath(const std::string& relativePath);

lua_State* L = nullptr;
GLFWwindow* g_window = nullptr;

void initLua() {
    L = luaL_newstate();   // Create Lua VM
    luaL_openlibs(L);      // Load Lua standard libraries
}

void shutdownLua() {
    lua_close(L);
}

// Lua function to load a texture and add it as a sprite
int LuaLoadTexture(lua_State* L) {
    const char* relativePath = luaL_checkstring(L, 1);
    float x = (float)luaL_optnumber(L, 2, 0.0);
    float y = (float)luaL_optnumber(L, 3, 0.0);
    float width = (float)luaL_optnumber(L, 4, 128.0);
    float height = (float)luaL_optnumber(L, 5, 128.0);

    // Resolve path using the engine's project folder
    std::string fullPath = relativePath;
    if (!assetFolder.empty()) {
        fullPath = AssetPath(relativePath);
    }

    GLuint tex = LoadTexture(fullPath.c_str()); // pass const char* to match header
    if (!tex) {
        lua_pushboolean(L, 0);
        return 1; // false on failure
    }

    sprites.push_back({tex, x, y, width, height});
    lua_pushboolean(L, 1);
    return 1; // true on success
}

int LuaMoveTexture(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);

    if (index < 0 || index >= (int)sprites.size()) {
        lua_pushboolean(L, 0);
        return 1; // false on invalid index
    }

    sprites[index].x = x;
    sprites[index].y = y;

    lua_pushboolean(L, 1);
    return 1; // true on success
}

int LuaIsKeyPressed(lua_State* L) {
    if (!g_window) {
        lua_pushboolean(L, false);
        return 1;
    }

    int key = luaL_checkinteger(L, 1);
    lua_pushboolean(L, glfwGetKey(g_window, key) == GLFW_PRESS);
    return 1;
}

void SetLuaWindow(GLFWwindow* window) {
    g_window = window;
}

void registerLuaFunctions() {
    lua_register(L, "LoadTexture", LuaLoadTexture);
    lua_register(L, "MoveTexture", LuaMoveTexture);
    lua_register(L, "IsKeyPressed", LuaIsKeyPressed);
}

bool RunLuaFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open Lua file: " << filepath << std::endl;
        return false;
    }

    std::string code((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());

    if (luaL_dostring(L, code.c_str()) != LUA_OK) {
        std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return false;
    }

    return true;
}
