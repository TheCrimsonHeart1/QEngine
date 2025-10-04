#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <GL/glew.h>
#include <string>

// The main function that takes a std::string (defined in .cpp)
GLuint LoadTexture(const std::string& filePath);

// Overload for const char* to simplify calls
inline GLuint LoadTexture(const char* filePath) {
    return LoadTexture(std::string(filePath));
}

#endif
