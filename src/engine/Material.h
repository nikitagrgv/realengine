#pragma once

#include "Base.h"
#include "Shader.h"

#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include <string>
#include <vector>

class Texture;
class ShaderSource;

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
    REMOVE_COPY_MOVE_CLASS(Material);

    Material();
    ~Material();

    void cloneTo(Material &dest) const;

    ShaderSource *getShaderSource() const;
    void setShaderSource(ShaderSource *source);
    void clearShaderSource();

    Shader *getShader();

    int addParameterFloat(const char *name);
    int addParameterFloat(const char *name, float value);
    void setParameterFloat(const char *name, float value);
    void setParameterFloat(int i, float value);
    float getParameterFloat(const char *name) const;
    float getParameterFloat(int i) const;

    int addParameterVec2(const char *name);
    int addParameterVec2(const char *name, glm::vec2 value);
    void setParameterVec2(const char *name, glm::vec2 value);
    void setParameterVec2(int i, glm::vec2 value);
    glm::vec2 getParameterVec2(const char *name) const;
    glm::vec2 getParameterVec2(int i) const;

    int addParameterVec3(const char *name);
    int addParameterVec3(const char *name, glm::vec3 value);
    void setParameterVec3(const char *name, glm::vec3 value);
    void setParameterVec3(int i, glm::vec3 value);
    glm::vec3 getParameterVec3(const char *name) const;
    glm::vec3 getParameterVec3(int i) const;

    int addParameterVec4(const char *name);
    int addParameterVec4(const char *name, glm::vec4 value);
    void setParameterVec4(const char *name, glm::vec4 value);
    void setParameterVec4(int i, glm::vec4 value);
    glm::vec4 getParameterVec4(const char *name) const;
    glm::vec4 getParameterVec4(int i) const;

    int addParameterMat4(const char *name);
    int addParameterMat4(const char *name, const glm::mat4 &value);
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

    int addTexture(const char *name);
    int addTexture(const char *name, Texture *texture);
    void setTexture(const char *name, Texture *texture);
    void setTexture(int i, Texture *texture);
    Texture *getTexture(int i) const { return textures_[i].texture; }
    const std::string &getTextureName(int i) const { return textures_[i].name; }
    int getNumTextures() const;
    void clearTextures();

    int addDefine(const char *name);
    int addDefine(const char *name, bool enabled);
    void setDefine(int i, bool enabled);
    void setDefine(const char *name, bool enabled);
    const std::string &getDefineName(int i) const { return defines_[i].name; }
    bool getDefine(int i) const;
    bool getDefine(const char *name) const;
    int getNumDefines() const;
    void clearDefines();

    bool isTwoSided() const { return two_sided_; }
    void setTwoSided(bool two_sided) { two_sided_ = two_sided; }

private:
    void set_defines_to_shader();

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

    struct Define
    {
        std::string name;
        bool enabled{false};
    };

private:
    int find_parameter(const char *name) const;
    int find_texture(const char *name) const;
    int find_define(const char *name) const;

private:
    std::vector<TextureInfo> textures_;
    std::vector<Parameter> parameters_;
    std::vector<Define> defines_;
    UPtr<Shader> shader_{};

    bool two_sided_{false};
};