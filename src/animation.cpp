#include "../include/animation.h"

std::vector<Animation> AnimationManager::animations;

Animation::Animation()
    : loop(true), playing(false), currentTime(0.0f), currentFrameIndex(0) {
}

void Animation::AddFrame(GLuint textureID, float duration) {
    frames.push_back({textureID, duration});
}

void Animation::Update(float deltaTime) {
    if (!playing || frames.empty()) {
        return;
    }

    currentTime += deltaTime;

    if (currentTime >= frames[currentFrameIndex].duration) {
        currentTime = 0.0f;
        currentFrameIndex++;

        if (currentFrameIndex >= (int)frames.size()) {
            if (loop) {
                currentFrameIndex = 0;
            } else {
                currentFrameIndex = (int)frames.size() - 1;
                playing = false;
            }
        }
    }
}

GLuint Animation::GetCurrentTexture() const {
    if (frames.empty()) {
        return 0;
    }
    return frames[currentFrameIndex].textureID;
}

void Animation::Play() {
    playing = true;
}

void Animation::Pause() {
    playing = false;
}

void Animation::Stop() {
    playing = false;
    Reset();
}

void Animation::Reset() {
    currentTime = 0.0f;
    currentFrameIndex = 0;
}

bool Animation::IsFinished() const {
    return !loop && currentFrameIndex >= (int)frames.size() - 1 && !playing;
}

// AnimationManager implementation
int AnimationManager::CreateAnimation(bool loop) {
    Animation anim;
    anim.loop = loop;
    animations.push_back(anim);
    return (int)animations.size() - 1;
}

bool AnimationManager::AddFrameToAnimation(int animIndex, GLuint textureID, float duration) {
    if (animIndex < 0 || animIndex >= (int)animations.size()) {
        return false;
    }
    animations[animIndex].AddFrame(textureID, duration);
    return true;
}

void AnimationManager::UpdateAnimation(int animIndex, float deltaTime) {
    if (animIndex < 0 || animIndex >= (int)animations.size()) {
        return;
    }
    animations[animIndex].Update(deltaTime);
}

void AnimationManager::PlayAnimation(int animIndex) {
    if (animIndex < 0 || animIndex >= (int)animations.size()) {
        return;
    }
    animations[animIndex].Play();
}

void AnimationManager::PauseAnimation(int animIndex) {
    if (animIndex < 0 || animIndex >= (int)animations.size()) {
        return;
    }
    animations[animIndex].Pause();
}

void AnimationManager::StopAnimation(int animIndex) {
    if (animIndex < 0 || animIndex >= (int)animations.size()) {
        return;
    }
    animations[animIndex].Stop();
}

void AnimationManager::ResetAnimation(int animIndex) {
    if (animIndex < 0 || animIndex >= (int)animations.size()) {
        return;
    }
    animations[animIndex].Reset();
}

GLuint AnimationManager::GetAnimationTexture(int animIndex) {
    if (animIndex < 0 || animIndex >= (int)animations.size()) {
        return 0;
    }
    return animations[animIndex].GetCurrentTexture();
}

bool AnimationManager::IsAnimationFinished(int animIndex) {
    if (animIndex < 0 || animIndex >= (int)animations.size()) {
        return true;
    }
    return animations[animIndex].IsFinished();
}

void AnimationManager::ClearAnimations() {
    animations.clear();
}