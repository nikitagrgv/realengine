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
        Mat4,

        NUM_TYPES
    };

    static const char *getParameterTypeName(ParameterType type);

public:
    REMOVE_COPY_MOVE_CLASS(Material);

    explicit Material(Material *parent = nullptr);
    ~Material();

    UPtr<Material> clone() const;
    UPtr<Material> inherit();

    Material *getBase() const { return base_mat_; }
    Material *getParent() const { return parent_mat_; }

    bool isBase() const
    {
        assert(base_mat_ != this || parent_mat_ != nullptr);
        return !parent_mat_;
    }

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
    const std::string &getParameterName(int i) const;
    int getNumParameters() const;
    void clearParameters();

    void setParameterOverriden(const char *name, bool overriden);
    void setParameterOverriden(int i, bool overriden);
    bool isParameterOverriden(const char *name) const;
    bool isParameterOverriden(int i) const;

    int addTexture(const char *name);
    int addTexture(const char *name, Texture *texture);
    void setTexture(const char *name, Texture *texture);
    void setTexture(int i, Texture *texture);
    Texture *getTexture(int i) const;
    const std::string &getTextureName(int i) const;
    int getNumTextures() const;
    void clearTextures();

    void setTextureOverriden(const char *name, bool overriden);
    void setTextureOverriden(int i, bool overriden);
    bool isTextureOverriden(const char *name) const;
    bool isTextureOverriden(int i) const;

    int addDefine(const char *name);
    int addDefine(const char *name, bool enabled);
    void setDefine(int i, bool enabled);
    void setDefine(const char *name, bool enabled);
    const std::string &getDefineName(int i) const;
    bool getDefine(int i) const;
    bool getDefine(const char *name) const;
    int getNumDefines() const;
    void clearDefines();

    void setDefineOverriden(const char *name, bool overriden);
    void setDefineOverriden(int i, bool overriden);
    bool isDefineOverriden(const char *name) const;
    bool isDefineOverriden(int i) const;

    bool isTwoSided() const;
    void setTwoSided(bool two_sided);
    void setTwoSidedOverriden(bool overriden);
    bool isTwoSidedOverriden() const;

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

    struct ParameterOverride
    {
        union
        {
            float float_value;
            glm::vec2 vec2_value;
            glm::vec3 vec3_value;
            glm::vec4 vec4_value;
            glm::mat4 mat4_value;
        };
        bool override{false};
    };

    struct TextureInfoOverride
    {
        Texture *texture{};
        bool override{false};
    };

    struct DefineOverride
    {
        bool enabled{false};
        bool override{false};
    };

    template<typename T>
    struct Option
    {
        explicit Option(T def = {})
            : value(def)
        {}

        T value{};
        bool override{false};
    };

private:
    void set_defines_to_shader();
    void on_define_overrides_changed();

    int find_parameter(const char *name) const;
    int find_texture(const char *name) const;
    int find_define(const char *name) const;

    static void make_child_parent(Material *child, Material *parent);
    static void unmake_child_parent(Material *child, Material *parent);

private:
    struct
    {
        std::vector<Parameter> parameters;
        std::vector<TextureInfo> textures;
        std::vector<Define> defines;
        // TODO: store all shader varitations here: map<defines, shader>
    } base_;

    struct
    {
        std::vector<ParameterOverride> parameters;
        std::vector<TextureInfoOverride> textures;
        std::vector<DefineOverride> defines;
    } inherited_;

    Material *base_mat_{};
    Material *parent_mat_{};
    std::vector<Material *> children_;

    UPtr<Shader> shader_{};

    struct
    {
        Option<bool> two_sided;
    } options_;
};