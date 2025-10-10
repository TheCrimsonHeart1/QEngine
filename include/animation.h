
#ifndef QENGINE_ANIMATION_H
#define QENGINE_ANIMATION_H

#include <vector>
#include <string>
#include <GL/glew.h>

struct AnimationFrame {
    GLuint textureID;
    float duration; // in seconds
};

class Animation {
public:
    std::vector<AnimationFrame> frames;
    bool loop;
    bool playing;
    float currentTime;
    int currentFrameIndex;

    Animation();
    void AddFrame(GLuint textureID, float duration);
    void Update(float deltaTime);
    GLuint GetCurrentTexture() const;
    void Play();
    void Pause();
    void Stop();
    void Reset();
    bool IsFinished() const;
};

class AnimationManager {
public:
    static std::vector<Animation> animations;

    static int CreateAnimation(bool loop = true);
    static bool AddFrameToAnimation(int animIndex, GLuint textureID, float duration);
    static void UpdateAnimation(int animIndex, float deltaTime);
    static void PlayAnimation(int animIndex);
    static void PauseAnimation(int animIndex);
    static void StopAnimation(int animIndex);
    static void ResetAnimation(int animIndex);
    static GLuint GetAnimationTexture(int animIndex);
    static bool IsAnimationFinished(int animIndex);
    static void ClearAnimations();
};

#endif //QENGINE_ANIMATION_H