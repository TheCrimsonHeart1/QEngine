#ifndef SPRITE_H
#define SPRITE_H

#include <GL/glew.h>

struct Sprite {
    GLuint textureID;
    float x, y;
    float width, height;
};

#endif // SPRITE_H