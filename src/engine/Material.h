#pragma once

#include "Base.h"

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

    static const char *getParameterTypeName(ParameterType type);

public:
    REMOVE_COPY_CLASS(Material);

    Material();
    ~Material();

    Material(Material &&other) noexcept;
    Material &operator=(Material &&other) noexcept;

    Material clone() const;

    void setShader(Shader *shader) { shader_ = shader; }
    Shader *getShader() const { return shader_; }
    void clearShader() { shader_ = nullptr; }

    void addParameterFloat(const char *name);
    void setParameterFloat(const char *name, float value);
    void setParameterFloat(int i, float value);
    float getParameterFloat(const char *name) const;
    float getParameterFloat(int i) const;

    void addParameterVec2(const char *name);
    void setParameterVec2(const char *name, glm::vec2 value);
    void setParameterVec2(int i, glm::vec2 value);
    glm::vec2 getParameterVec2(const char *name) const;
    glm::vec2 getParameterVec2(int i) const;

    void addParameterVec3(const char *name);
    void setParameterVec3(const char *name, glm::vec3 value);
    void setParameterVec3(int i, glm::vec3 value);
    glm::vec3 getParameterVec3(const char *name) const;
    glm::vec3 getParameterVec3(int i) const;

    void addParameterVec4(const char *name);
    void setParameterVec4(const char *name, glm::vec4 value);
    void setParameterVec4(int i, glm::vec4 value);
    glm::vec4 getParameterVec4(const char *name) const;
    glm::vec4 getParameterVec4(int i) const;

    void addParameterMat4(const char *name);
    void setParameterMat4(const char *name, const glm::mat4 &value);
    void setParameterMat4(int i, const glm::mat4 &value);
    glm::mat4 getParameterMat4(const char *name) const;
    glm::mat4 getParameterMat4(int i) const;

    bool hasParameter(const char *name) const;
    ParameterType getParameterType(int i) const;
    const char *getParameterTypeName(int i) const;
    const std::string &getParameterName(int i) const { return parameters_[i].name; }
    int getNumParameters() const;
    void clearParameters();

    void addTexture(const char *name);
    void setTexture(const char *name, Texture *texture);
    Texture *getTexture(int i) const { return textures_[i].texture; }
    const std::string &getTextureName(int i) const { return textures_[i].name; }
    int getNumTextures() const;
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