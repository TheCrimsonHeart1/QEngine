#include "../include/TextureLoader.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <GL/glew.h> // or glad

// For stb_image
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Define a function to get OpenGL format from channels
GLenum get_gl_format(int channels) {
    switch (channels) {
        case 4: return GL_RGBA;
        case 3: return GL_RGB;
        default: return 0;
    }
}

// Corrected version to fix color inversion
GLuint LoadTexture(const std::string& filePath) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Check file extension to decide which library to use
    if (filePath.ends_with(".png")) {
        // Use stb_image for PNGs
        int width, height, channels;
        unsigned char* imageData = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

        if (!imageData) {
            std::cerr << "stb_image Error (" << filePath << "): " << stbi_failure_reason() << std::endl;
            glDeleteTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, 0);
            return 0;
        }

        GLenum format = get_gl_format(channels);
        if (format == 0) {
            std::cerr << "Unsupported PNG channel count (" << filePath << "): " << channels << std::endl;
            stbi_image_free(imageData);
            glDeleteTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, 0);
            return 0;
        }

        // Fix for non-4-byte aligned textures (RGB images)
        if (channels == 3) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }

        // Upload to GPU
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(imageData);

        // Restore default alignment
        if (channels == 3) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        }

    } else if (filePath.ends_with(".bmp")) {
        // Use SDL_image for BMPs (this requires an intermediate surface)
        SDL_Surface* surface = IMG_Load(filePath.c_str());
        if (!surface) {
            glDeleteTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, 0);
            return 0;
        }

        // Determine the correct OpenGL format from the surface
        GLenum format;
        GLint internalFormat;
        if (surface->format == SDL_PIXELFORMAT_BGRA32 || surface->format == SDL_PIXELFORMAT_BGRX32) {
            format = GL_BGRA;
            internalFormat = GL_RGBA8;
        } else if (surface->format == SDL_PIXELFORMAT_RGBA32 || surface->format == SDL_PIXELFORMAT_RGBX32) {
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
        } else if (surface->format == SDL_PIXELFORMAT_BGR24) {
            format = GL_BGR;
            internalFormat = GL_RGB8;
        } else {
            // For other formats, convert to a known one first
            SDL_Surface* convertedSurface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
            SDL_DestroySurface(surface);
            if (!convertedSurface) {
                std::cerr << "BMP conversion error: " << SDL_GetError() << std::endl;
                glDeleteTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, 0);
                return 0;
            }
            surface = convertedSurface;
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
        }

        // Upload to GPU
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
        glGenerateMipmap(GL_TEXTURE_2D);

        SDL_DestroySurface(surface);

    } else {
        std::cerr << "Unsupported file type: " << filePath << std::endl;
        glDeleteTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, 0);
        return 0;
    }

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
