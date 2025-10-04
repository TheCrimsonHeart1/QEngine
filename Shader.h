#pragma once
#include <string>
#include <GL/glew.h>

class Shader {
public:
    GLuint ID;

    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    void use() const { glUseProgram(ID); }
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setMat4(const std::string &name, const float* mat) const;
    void setVec4(const std::string &name, float x, float y, float z, float w) const;  // ADD THIS

private:
    std::string loadFile(const std::string& path);
    GLuint compileShader(GLenum type, const std::string& source);
};