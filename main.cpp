#include <iostream>
#include <vector>
#include <map>
#include <filesystem>

// OpenGL
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// SDL3
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Lua
extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

// Project headers
#include "TextureLoader.h"
#include "UI.h"
#include "Sprite.h"
#include "Shader.h"
#include "CodeEditor.h"
#include "LuaScripting.h"
#include "AssetManager.h"

// Global state
CodeEditor luaEditor;
std::vector<Sprite> sprites;

void processInput(GLFWwindow* window, std::map<int, bool>& keyPresses) {
    for (int key = GLFW_KEY_SPACE; key < GLFW_KEY_LAST; key++) {
        if (glfwGetKey(window, key) == GLFW_PRESS && !keyPresses[key]) {
            keyPresses[key] = true;
        } else if (glfwGetKey(window, key) == GLFW_RELEASE) {
            keyPresses[key] = false;
        }
    }
}

void updateLua(float deltaTime) {
    // Call Lua Update function if it exists
    lua_getglobal(L, "Update");
    if (lua_type(L, -1) == LUA_TFUNCTION) {
        lua_pushnumber(L, deltaTime);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            std::cerr << "Lua Update error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }

    // Resume coroutine if it exists
    lua_getglobal(L, "MoveCoroutine");
    if (lua_type(L, -1) == LUA_TTHREAD) {
        lua_State* co = lua_tothread(L, -1);
        int nresults = 0;
        int status = lua_resume(co, nullptr, 0, &nresults);
        if (status != LUA_OK && status != LUA_YIELD) {
            std::cerr << "Coroutine error: " << lua_tostring(co, -1) << std::endl;
            lua_pop(co, 1);
        }
    }
    lua_pop(L, 1);
}

bool initializeOpenGL(GLFWwindow*& window) {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Set OpenGL version hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    window = glfwCreateWindow(1920, 1080, "QEngine - 2D Game Engine", glfwGetPrimaryMonitor(), nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }

    // Clear any GLEW errors
    glGetError();

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::cout << "OpenGL initialized successfully" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    return true;
}

bool setupQuadGeometry(GLuint& VAO, GLuint& VBO, GLuint& EBO) {
    // Quad vertices: position (2) + texcoords (2)
    float vertices[] = {
        // positions        // texture coords
        -0.5f, -0.5f,       0.0f, 0.0f,  // bottom-left
         0.5f, -0.5f,       1.0f, 0.0f,  // bottom-right
         0.5f,  0.5f,       1.0f, 1.0f,  // top-right
        -0.5f,  0.5f,       0.0f, 1.0f   // top-left
    };

    unsigned int indices[] = {
        0, 1, 2,   // first triangle
        2, 3, 0    // second triangle
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Check for errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error during quad setup: " << error << std::endl;
        return false;
    }

    std::cout << "Quad geometry setup complete" << std::endl;
    return true;
}

bool initializeImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cerr << "Failed to initialize ImGui GLFW backend" << std::endl;
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        std::cerr << "Failed to initialize ImGui OpenGL3 backend" << std::endl;
        return false;
    }

    std::cout << "ImGui initialized successfully" << std::endl;
    return true;
}

void renderSprites(Shader& shader, GLuint VAO, const std::vector<Sprite>& sprites) {
    shader.use();
    glBindVertexArray(VAO);

    for (const auto& sprite : sprites) {
        // Create model matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(sprite.x, sprite.y, 0.0f));
        model = glm::scale(model, glm::vec3(sprite.width, sprite.height, 1.0f));

        shader.setMat4("model", glm::value_ptr(model));

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sprite.textureID);
        shader.setInt("spriteTexture", 0);  // Use 'shader' not 'spriteShader'

        // Draw
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}

void cleanup(GLuint VAO, GLuint VBO, GLuint EBO, std::vector<Sprite>& sprites) {
    // Delete all sprite textures
    for (auto& sprite : sprites) {
        glDeleteTextures(1, &sprite.textureID);
    }
    sprites.clear();

    // Delete OpenGL objects
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // Shutdown ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Shutdown Lua
    shutdownLua();

    std::cout << "Cleanup complete" << std::endl;
}

int main(int argc, char** argv) {
    // Set asset folder from command line argument
    if (argc > 1) {
        assetFolder = argv[1];
        std::cout << "Asset folder set to: " << assetFolder << std::endl;
    } else {
        std::cout << "Using default asset folder: " << assetFolder << std::endl;
    }

    // Initialize OpenGL and create window
    GLFWwindow* window = nullptr;
    if (!initializeOpenGL(window)) {
        return -1;
    }

    // Setup quad geometry
    GLuint VAO, VBO, EBO;
    if (!setupQuadGeometry(VAO, VBO, EBO)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Load and compile shaders
    Shader spriteShader("sprite.vert", "sprite.frag");
    spriteShader.use();

    // Set up orthographic projection (screen coordinates)
    glm::mat4 projection = glm::ortho(0.0f, 1920.0f, 1080.0f, 0.0f, -1.0f, 1.0f);
    spriteShader.setMat4("projection", glm::value_ptr(projection));

    // Set default sprite color (white = no tint)
    spriteShader.setVec4("spriteColor", 1.0f, 1.0f, 1.0f, 1.0f);

    std::cout << "Shaders loaded and projection matrix set" << std::endl;
    // Initialize ImGui
    if (!initializeImGui(window)) {
        cleanup(VAO, VBO, EBO, sprites);
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Initialize Lua
    initLua();
    registerLuaFunctions();
    SetLuaWindow(window);

    std::cout << "All systems initialized. Starting main loop..." << std::endl;
    #if !GAME_MODE

    std::string mainScriptPath = (assetFolder / "scripts" / "main.lua").string();

    std::cout << "Running game script: " << mainScriptPath << std::endl;

    if (luaL_dofile(L, mainScriptPath.c_str()) != LUA_OK) {
        std::cerr << "Lua runtime error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }
    #endif
    // Setup input tracking
    std::map<int, bool> keyPresses;

    // Asset folder input buffer
    static char folderInput[512];
    strncpy(folderInput, assetFolder.string().c_str(), sizeof(folderInput) - 1);
    folderInput[sizeof(folderInput) - 1] = '\0';

    // Timing
    double lastTime = glfwGetTime();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // Process input
        processInput(window, keyPresses);

        // Update Lua scripts
        updateLua(deltaTime);

        // Clear screen
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render sprites
        renderSprites(spriteShader, VAO, sprites);
        #if GAME_MODE
        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render GUI
        RenderGUI(sprites);
        RenderCodeEditor(luaEditor, "Lua Script Editor");

        // Run script button
        if (ImGui::Button("Run Script")) {
            std::string code = luaEditor.editor.GetText();
            if (luaL_dostring(L, code.c_str()) != LUA_OK) {
                std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl;
                lua_pop(L, 1);
            }
        }

        // Project path input
        ImGui::InputText("Project Path", folderInput, sizeof(folderInput));
        if (ImGui::Button("Set Project Path")) {
            assetFolder = folderInput;
            std::cout << "Project path set to: " << assetFolder << std::endl;
        }

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        #endif

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    cleanup(VAO, VBO, EBO, sprites);
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Engine shut down successfully" << std::endl;
    return 0;
}