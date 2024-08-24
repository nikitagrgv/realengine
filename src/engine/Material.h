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

    void setTexture(const char *name, Texture *texture)
    {
        for (int i = 0; i < textures_.size(); i++)
        {
            if (textures_[i].name == name)
            {
                textures_[i].texture = texture;
                return;
            }
        }
        textures_.push_back({name, texture});
    }

    Texture *getTexture(int i) { return textures_[i].texture; }
    std::string &getTextureName(int i) { return textures_[i].name; }
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

    struct TextureInfo
    {
        std::string name;
        Texture *texture;
    };

private:
    int find_parameter(const char *name) const;

private:
    std::vector<TextureInfo> textures_;
    std::vector<Parameter> parameters_;
    Shader *shader_{};
};