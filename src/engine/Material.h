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
        Vec3,
        Vec4,
        Mat4
    };

public:
    void setShader(Shader *shader) { shader_ = shader; }
    Shader *getShader() const { return shader_; }
    void clearShader() { shader_ = nullptr; }

    void setParameterFloat(const char *name, float value);
    float getParameterFloat(int i);

    void setParameterVec3(const char *name, glm::vec3 value);
    glm::vec3 getParameterVec3(int i);

    void setParameterVec4(const char *name, glm::vec4 value);
    glm::vec4 getParameterVec4(int i);

    void setParameterMat4(const char *name, const glm::mat4 &value);
    glm::mat4 getParameterMat4(int i);

    ParameterType getParameterType(int i);
    std::string getParameterName(int i) { return parameters_[i].name; }
    int getNumParameters();
    void clearParameters();

    void setTexture(int i, Texture *texture)
    {
        if (i >= textures_.size())
        {
            textures_.resize(i + 1, nullptr);
        }
        textures_[i] = texture;
    }

    Texture *getTexture(int i) const { return textures_[i]; }
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
            glm::vec3 vec3_value;
            glm::vec4 vec4_value;
            glm::mat4 mat4_value;
        };
    };

private:
    int find_parameter(const char *name) const;

private:
    std::vector<Texture *> textures_;
    std::vector<Parameter> parameters_;
    Shader *shader_{};
};