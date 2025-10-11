extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include "../include/TextureLoader.h"
#include "../include/animation.h"
#include <vector>
#include "../include/Sprite.h"
#include <GLFW/glfw3.h>
#include "../include/LuaScripting.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include "../include/Collision.h"
#include <SDL3/SDL.h>

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

int LuaCheckCollision(lua_State* L) {
    int indexA = (int)luaL_checkinteger(L, 1);
    int indexB = (int)luaL_checkinteger(L, 2);

    if (indexA < 0 || indexA >= (int)sprites.size() ||
        indexB < 0 || indexB >= (int)sprites.size()) {
        lua_pushboolean(L, false);
        return 1;
        }

    bool colliding = CollisionManager::CheckCollision(sprites[indexA], sprites[indexB]);
    lua_pushboolean(L, colliding);
    return 1;
}
int LuaFindCollision(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);

    if (index < 0 || index >= (int)sprites.size()) {
        lua_pushinteger(L, -1);
        return 1;
    }

    int collision = CollisionManager::FindFirstCollision(sprites[index], sprites, index);
    lua_pushinteger(L, collision);
    return 1;
}
int LuaFindAllCollisions(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);

    if (index < 0 || index >= (int)sprites.size()) {
        lua_newtable(L);
        return 1;
    }

    std::vector<int> collisions = CollisionManager::FindAllCollisions(sprites[index], sprites, index);

    lua_newtable(L);
    for (size_t i = 0; i < collisions.size(); i++) {
        lua_pushinteger(L, collisions[i]);
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}
int LuaPointInSprite(lua_State* L) {
    float x = (float)luaL_checknumber(L, 1);
    float y = (float)luaL_checknumber(L, 2);
    int index = (int)luaL_checkinteger(L, 3);

    if (index < 0 || index >= (int)sprites.size()) {
        lua_pushboolean(L, false);
        return 1;
    }

    bool inside = CollisionManager::PointInSprite(x, y, sprites[index]);
    lua_pushboolean(L, inside);
    return 1;
}
int LuaResolveCollision(lua_State* L) {
    int indexA = (int)luaL_checkinteger(L, 1);
    int indexB = (int)luaL_checkinteger(L, 2);

    if (indexA < 0 || indexA >= (int)sprites.size() ||
        indexB < 0 || indexB >= (int)sprites.size()) {
        lua_pushboolean(L, false);
        return 1;
        }

    CollisionInfo info;
    if (CollisionManager::CheckCollision(sprites[indexA], sprites[indexB], info)) {
        CollisionManager::ResolveCollision(sprites[indexA], sprites[indexB], info);
        lua_pushboolean(L, true);
    } else {
        lua_pushboolean(L, false);
    }

    return 1;
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

int ChangeTexture(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);
    const char* relativePath = luaL_checkstring(L, 2);

    // Validate sprite index
    if (index < 0 || index >= (int)sprites.size()) {
        lua_pushboolean(L, 0);
        return 1; // false on invalid index
    }

    std::string fullPath = relativePath;
    if (!assetFolder.empty()) {
        fullPath = AssetPath(relativePath);
    }

    GLuint newTex = LoadTexture(fullPath.c_str());
    if (!newTex) {
        lua_pushboolean(L, 0);
        return 1; // false on failure
    }

    // Delete old texture (optional, prevents memory leaks)
    glDeleteTextures(1, &sprites[index].textureID);


    sprites[index].textureID = newTex;

    lua_pushboolean(L, 1);
    return 1;
}


void SetLuaWindow(GLFWwindow* window) {
    g_window = window;
}

int LuaCreateAnimation(lua_State* L) {
    bool loop = lua_toboolean(L, 1);
    int animIndex = AnimationManager::CreateAnimation(loop);
    lua_pushinteger(L, animIndex);
    return 1;
}

int LuaAddAnimationFrame(lua_State* L) {
    int animIndex = (int)luaL_checkinteger(L, 1);
    const char* relativePath = luaL_checkstring(L, 2);
    float duration = (float)luaL_checknumber(L, 3);

    std::string fullPath = relativePath;
    if (!assetFolder.empty()) {
        fullPath = AssetPath(relativePath);
    }

    GLuint tex = LoadTexture(fullPath.c_str());
    if (!tex) {
        lua_pushboolean(L, false);
        return 1;
    }

    bool success = AnimationManager::AddFrameToAnimation(animIndex, tex, duration);
    lua_pushboolean(L, success);
    return 1;
}

int LuaUpdateAnimation(lua_State* L) {
    int animIndex = (int)luaL_checkinteger(L, 1);
    float deltaTime = (float)luaL_checknumber(L, 2);
    AnimationManager::UpdateAnimation(animIndex, deltaTime);
    return 0;
}

int LuaPlayAnimation(lua_State* L) {
    int animIndex = (int)luaL_checkinteger(L, 1);
    AnimationManager::PlayAnimation(animIndex);
    return 0;
}

int LuaPauseAnimation(lua_State* L) {
    int animIndex = (int)luaL_checkinteger(L, 1);
    AnimationManager::PauseAnimation(animIndex);
    return 0;
}

int LuaStopAnimation(lua_State* L) {
    int animIndex = (int)luaL_checkinteger(L, 1);
    AnimationManager::StopAnimation(animIndex);
    return 0;
}

int LuaResetAnimation(lua_State* L) {
    int animIndex = (int)luaL_checkinteger(L, 1);
    AnimationManager::ResetAnimation(animIndex);
    return 0;
}

int LuaGetAnimationTexture(lua_State* L) {
    int animIndex = (int)luaL_checkinteger(L, 1);
    GLuint textureID = AnimationManager::GetAnimationTexture(animIndex);
    lua_pushinteger(L, textureID);
    return 1;
}

int LuaIsAnimationFinished(lua_State* L) {
    int animIndex = (int)luaL_checkinteger(L, 1);
    bool finished = AnimationManager::IsAnimationFinished(animIndex);
    lua_pushboolean(L, finished);
    return 1;
}

int LuaSetSpriteAnimation(lua_State* L) {
    int spriteIndex = (int)luaL_checkinteger(L, 1);
    int animIndex = (int)luaL_checkinteger(L, 2);

    if (spriteIndex < 0 || spriteIndex >= (int)sprites.size()) {
        lua_pushboolean(L, false);
        return 1;
    }

    GLuint textureID = AnimationManager::GetAnimationTexture(animIndex);
    if (textureID == 0) {
        lua_pushboolean(L, false);
        return 1;
    }

    sprites[spriteIndex].textureID = textureID;
    lua_pushboolean(L, true);
    return 1;
}
// Change sprite texture directly using GLuint
int LuaSetSpriteTexture(lua_State* L) {
    int spriteIndex = (int)luaL_checkinteger(L, 1);
    GLuint texID = (GLuint)luaL_checkinteger(L, 2);

    if (spriteIndex < 0 || spriteIndex >= (int)sprites.size()) {
        lua_pushboolean(L, false);
        return 1;
    }

    // Delete old texture if needed
    glDeleteTextures(1, &sprites[spriteIndex].textureID);
    sprites[spriteIndex].textureID = texID;

    lua_pushboolean(L, true);
    return 1;
}
int LuaGetSpritePosition(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);
    if (index < 0 || index >= (int)sprites.size()) {
        lua_pushnil(L);
        return 1;
    }
    lua_newtable(L);
    lua_pushnumber(L, sprites[index].x);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, sprites[index].y);
    lua_rawseti(L, -2, 2);
    return 1;
}
int LuaSetSpriteSize(lua_State* L) {
    int index = (int)luaL_checkinteger(L, 1);
    float width = (float)luaL_checknumber(L, 2);
    float height = (float)luaL_checknumber(L, 3);

    if (index < 0 || index >= (int)sprites.size()) {
        lua_pushboolean(L, false);
        return 1;
    }

    sprites[index].width = width;
    sprites[index].height = height;
    lua_pushboolean(L, true);
    return 1;
}


// ... existing code ...

void registerLuaFunctions() {
    lua_register(L, "GetSpritePosition", LuaGetSpritePosition);
    lua_register(L, "LoadTexture", LuaLoadTexture);
    lua_register(L, "MoveTexture", LuaMoveTexture);
    lua_register(L, "IsKeyPressed", LuaIsKeyPressed);
    lua_register(L, "ChangeTexture", ChangeTexture);
    lua_register(L, "SetSpriteTexture", LuaSetSpriteTexture);
    lua_register(L, "SetSpriteSize", LuaSetSpriteSize);

    lua_register(L, "CheckCollision", LuaCheckCollision);
    lua_register(L, "FindCollision", LuaFindCollision);
    lua_register(L, "FindAllCollisions", LuaFindAllCollisions);
    lua_register(L, "PointInSprite", LuaPointInSprite);
    lua_register(L, "ResolveCollision", LuaResolveCollision);

    // Animation functions
    lua_register(L, "CreateAnimation", LuaCreateAnimation);
    lua_register(L, "AddAnimationFrame", LuaAddAnimationFrame);
    lua_register(L, "UpdateAnimation", LuaUpdateAnimation);
    lua_register(L, "PlayAnimation", LuaPlayAnimation);
    lua_register(L, "PauseAnimation", LuaPauseAnimation);
    lua_register(L, "StopAnimation", LuaStopAnimation);
    lua_register(L, "ResetAnimation", LuaResetAnimation);
    lua_register(L, "GetAnimationTexture", LuaGetAnimationTexture);
    lua_register(L, "IsAnimationFinished", LuaIsAnimationFinished);
    lua_register(L, "SetSpriteAnimation", LuaSetSpriteAnimation);
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
