#pragma once
#include <string>
#include <GLFW/glfw3.h>

// Forward declare lua_State
struct lua_State;

extern lua_State* L;

void initLua();
void shutdownLua();
void registerLuaFunctions();
void SetLuaWindow(GLFWwindow* window);

// Lua functions exposed to C++ (optional)
bool RunLuaFile(const std::string& filepath);
