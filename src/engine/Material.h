#pragma once

#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include <string>
#include <vector>

class Texture;
class Shader;

class Material
{
public:
    enum class ParameterType
    {
        Float,
        Vec2,
        Vec3,
        Vec4,
        Mat4
    };

public:
    void setShader(Shader *shader) { shader_ = shader; }
    Shader *getShader() const { return shader_; }
    void clearShader() { shader_ = nullptr; }

    void addParameterFloat(const char *name);
    void setParameterFloat(const char *name, float value);
    float getParameterFloat(const char *name);
    float getParameterFloat(int i);

    void addParameterVec2(const char *name);
    void setParameterVec2(const char *name, glm::vec2 value);
    glm::vec2 getParameterVec2(const char *name);
    glm::vec2 getParameterVec2(int i);

    void addParameterVec3(const char *name);
    void setParameterVec3(const char *name, glm::vec3 value);
    glm::vec3 getParameterVec3(const char *name);
    glm::vec3 getParameterVec3(int i);

    void addParameterVec4(const char *name);
    void setParameterVec4(const char *name, glm::vec4 value);
    glm::vec4 getParameterVec4(const char *name);
    glm::vec4 getParameterVec4(int i);

    void addParameterMat4(const char *name);
    void setParameterMat4(const char *name, const glm::mat4 &value);
    glm::mat4 getParameterMat4(const char *name);
    glm::mat4 getParameterMat4(int i);

    ParameterType getParameterType(int i);
    std::string getParameterName(int i) { return parameters_[i].name; }
    int getNumParameters();
    void clearParameters();


    void addTexture(const char *name);
    void setTexture(const char *name, Texture *texture);
    Texture *getTexture(int i) const { return textures_[i].texture; }
    const std::string &getTextureName(int i) const { return textures_[i].name; }
    int getNumTextures();
    void clearTextures();

private:
    struct Parameter
    {
        std::string name;
        ParameterType type;
        union
        {
            float float_value;
            glm::vec2 vec2_value;
            glm::vec3 vec3_value;
            glm::vec4 vec4_value;
            glm::mat4 mat4_value;
        };
    };

    struct TextureInfo
    {
        std::string name;
        Texture *texture{};
    };

private:
    int find_parameter(const char *name) const;
    int find_texture(const char *name) const;

private:
    std::vector<TextureInfo> textures_;
    std::vector<Parameter> parameters_;
    Shader *shader_{};
};