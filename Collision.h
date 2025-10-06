#pragma once
#include "Sprite.h"
#include <vector>

// Axis-Aligned Bounding Box structure
struct AABB {
    float x, y;          // Top-left corner (matching your Sprite structure)
    float width, height;
    
    AABB() : x(0), y(0), width(0), height(0) {}
    AABB(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}
    
    // Create AABB from sprite
    static AABB FromSprite(const Sprite& sprite) {
        return AABB(sprite.x, sprite.y, sprite.width, sprite.height);
    }
    
    // Check if this AABB intersects with another
    bool Intersects(const AABB& other) const {
        return (x < other.x + other.width &&
                x + width > other.x &&
                y < other.y + other.height &&
                y + height > other.y);
    }
    
    // Check if a point is inside this AABB
    bool ContainsPoint(float px, float py) const {
        return (px >= x && px <= x + width &&
                py >= y && py <= y + height);
    }
    
    // Get center point
    void GetCenter(float& cx, float& cy) const {
        cx = x + width * 0.5f;
        cy = y + height * 0.5f;
    }
};

// Collision info structure
struct CollisionInfo {
    int spriteA;        // Index of first sprite
    int spriteB;        // Index of second sprite
    float overlapX;     // Overlap on X axis
    float overlapY;     // Overlap on Y axis
    
    CollisionInfo() : spriteA(-1), spriteB(-1), overlapX(0), overlapY(0) {}
    CollisionInfo(int a, int b, float ox, float oy) 
        : spriteA(a), spriteB(b), overlapX(ox), overlapY(oy) {}
};

// Collision manager class - all static methods for easy use
class CollisionManager {
public:
    // Basic collision check between two sprites
    static bool CheckCollision(const Sprite& a, const Sprite& b);
    
    // Check collision and get detailed info
    static bool CheckCollision(const Sprite& a, const Sprite& b, CollisionInfo& info);
    
    // Check collision between two AABBs
    static bool CheckCollision(const AABB& a, const AABB& b);
    
    // Find first sprite that collides with given sprite
    static int FindFirstCollision(const Sprite& sprite, const std::vector<Sprite>& sprites, int ignoreIndex = -1);
    
    // Get all sprites that collide with given sprite
    static std::vector<int> FindAllCollisions(const Sprite& sprite, const std::vector<Sprite>& sprites, int ignoreIndex = -1);
    
    // Get all collision pairs in sprite list
    static std::vector<CollisionInfo> GetAllCollisions(const std::vector<Sprite>& sprites);
    
    // Resolve collision by pushing sprites apart
    static void ResolveCollision(Sprite& a, Sprite& b, const CollisionInfo& info);
    
    // Check if point is inside sprite
    static bool PointInSprite(float x, float y, const Sprite& sprite);
};