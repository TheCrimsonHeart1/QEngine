#include "Collision.h"
#include <algorithm>
#include <cmath>

// Basic collision check
bool CollisionManager::CheckCollision(const Sprite& a, const Sprite& b) {
    return (a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y);
}

// Collision check with detailed info
bool CollisionManager::CheckCollision(const Sprite& a, const Sprite& b, CollisionInfo& info) {
    if (!CheckCollision(a, b)) {
        return false;
    }

    // Calculate overlap on each axis
    float leftOverlap = (a.x + a.width) - b.x;
    float rightOverlap = (b.x + b.width) - a.x;
    float topOverlap = (a.y + a.height) - b.y;
    float bottomOverlap = (b.y + b.height) - a.y;

    // Find minimum overlap on each axis
    info.overlapX = std::min(leftOverlap, rightOverlap);
    info.overlapY = std::min(topOverlap, bottomOverlap);

    return true;
}

// AABB collision check
bool CollisionManager::CheckCollision(const AABB& a, const AABB& b) {
    return a.Intersects(b);
}

// Find first collision
int CollisionManager::FindFirstCollision(const Sprite& sprite, const std::vector<Sprite>& sprites, int ignoreIndex) {
    for (size_t i = 0; i < sprites.size(); i++) {
        if (static_cast<int>(i) == ignoreIndex) {
            continue;
        }

        if (CheckCollision(sprite, sprites[i])) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

// Find all collisions
std::vector<int> CollisionManager::FindAllCollisions(const Sprite& sprite, const std::vector<Sprite>& sprites, int ignoreIndex) {
    std::vector<int> collisions;

    for (size_t i = 0; i < sprites.size(); i++) {
        if (static_cast<int>(i) == ignoreIndex) {
            continue;
        }

        if (CheckCollision(sprite, sprites[i])) {
            collisions.push_back(static_cast<int>(i));
        }
    }

    return collisions;
}

// Get all collision pairs
std::vector<CollisionInfo> CollisionManager::GetAllCollisions(const std::vector<Sprite>& sprites) {
    std::vector<CollisionInfo> collisions;

    for (size_t i = 0; i < sprites.size(); i++) {
        for (size_t j = i + 1; j < sprites.size(); j++) {
            CollisionInfo info;
            if (CheckCollision(sprites[i], sprites[j], info)) {
                info.spriteA = static_cast<int>(i);
                info.spriteB = static_cast<int>(j);
                collisions.push_back(info);
            }
        }
    }

    return collisions;
}

// Resolve collision by pushing sprites apart
void CollisionManager::ResolveCollision(Sprite& a, Sprite& b, const CollisionInfo& info) {
    // Push sprites apart on the axis with least overlap
    if (info.overlapX < info.overlapY) {
        // Separate on X axis
        float separationX = info.overlapX * 0.5f;
        if (a.x < b.x) {
            a.x -= separationX;
            b.x += separationX;
        } else {
            a.x += separationX;
            b.x -= separationX;
        }
    } else {
        // Separate on Y axis
        float separationY = info.overlapY * 0.5f;
        if (a.y < b.y) {
            a.y -= separationY;
            b.y += separationY;
        } else {
            a.y += separationY;
            b.y -= separationY;
        }
    }
}

// Point in sprite check
bool CollisionManager::PointInSprite(float x, float y, const Sprite& sprite) {
    return (x >= sprite.x && x <= sprite.x + sprite.width &&
            y >= sprite.y && y <= sprite.y + sprite.height);
}