#include "../include/TextureLoader.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <GL/glew.h> // or glad

GLuint LoadTexture(const std::string& filePath) {
    // Load image
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    if (!surface) {
        std::cerr << "IMG_Load Error (" << filePath << "): " << SDL_GetError() << std::endl;
        return 0;
    }

    // Determine format
    GLenum format;
    GLint internalFormat;
    if (SDL_ISPIXELFORMAT_ALPHA(surface->format)) {
        format = GL_RGBA;
        internalFormat = GL_RGBA8;
    } else {
        format = GL_RGB;
        internalFormat = GL_RGB8;
    }

    // Convert to known pixel format
    SDL_Surface* formattedSurface = (format == GL_RGBA)
        ? SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32)
        : SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGB24);
    SDL_DestroySurface(surface);

    if (!formattedSurface) {
        std::cerr << "Surface conversion error: " << SDL_GetError() << std::endl;
        return 0;
    }

    // Generate OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload to GPU
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                 formattedSurface->w, formattedSurface->h,
                 0, format, GL_UNSIGNED_BYTE, formattedSurface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_DestroySurface(formattedSurface);

    // Error check
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL Error (" << filePath << "): " << error << std::endl;
        glDeleteTextures(1, &textureID);
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}
