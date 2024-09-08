#include "Material.h"

#include <iostream>

#define DEFINE_PARAMTERS_METHODS(TYPE_NAME, TYPE_VALUE_GET, TYPE_VALUE_SET, UNION_ELEMENT,         \
    DEFAULT_VALUE)                                                                                 \
                                                                                                   \
    int Material::addParameter##TYPE_NAME(const char *name)                                        \
    {                                                                                              \
        return addParameter##TYPE_NAME(name, DEFAULT_VALUE);                                       \
    }                                                                                              \
                                                                                                   \
    int Material::addParameter##TYPE_NAME(const char *name, TYPE_VALUE_SET value)                  \
    {                                                                                              \
        const int index = find_parameter(name);                                                    \
        if (index != -1)                                                                           \
        {                                                                                          \
            std::cout << "Parameter already exists: " << name << std::endl;                        \
            return -1;                                                                             \
        }                                                                                          \
        Parameter parameter;                                                                       \
        parameter.name = name;                                                                     \
        parameter.type = ParameterType::##TYPE_NAME;                                               \
        parameter.##UNION_ELEMENT##_value = value;                                                 \
        const int i = parameters_.size();                                                          \
        parameters_.push_back(parameter);                                                          \
        return i;                                                                                  \
    }                                                                                              \
                                                                                                   \
    void Material::setParameter##TYPE_NAME(const char *name, TYPE_VALUE_SET value)                 \
    {                                                                                              \
        const int index = find_parameter(name);                                                    \
        if (index == -1)                                                                           \
        {                                                                                          \
            std::cout << "Parameter not found: " << name << std::endl;                             \
            return;                                                                                \
        }                                                                                          \
        if (parameters_[index].type != ParameterType::##TYPE_NAME)                                 \
        {                                                                                          \
            std::cout << "Parameter type does not match: " << name << std::endl;                   \
            return;                                                                                \
        }                                                                                          \
        parameters_[index].##UNION_ELEMENT##_value = value;                                        \
    }                                                                                              \
                                                                                                   \
    void Material::setParameter##TYPE_NAME(int i, TYPE_VALUE_SET value)                            \
    {                                                                                              \
        if (parameters_[i].type != ParameterType::##TYPE_NAME)                                     \
        {                                                                                          \
            std::cout << "Parameter type does not match: " << i << std::endl;                      \
            return;                                                                                \
        }                                                                                          \
        parameters_[i].##UNION_ELEMENT##_value = value;                                            \
    }                                                                                              \
                                                                                                   \
    TYPE_VALUE_GET Material::getParameter##TYPE_NAME(const char *name) const                       \
    {                                                                                              \
        const int index = find_parameter(name);                                                    \
        if (index == -1)                                                                           \
        {                                                                                          \
            std::cout << "Parameter not found: " << name << std::endl;                             \
            return DEFAULT_VALUE;                                                                  \
        }                                                                                          \
        return getParameter##TYPE_NAME(index);                                                     \
    }                                                                                              \
                                                                                                   \
    TYPE_VALUE_GET Material::getParameter##TYPE_NAME(int i) const                                  \
    {                                                                                              \
        if (parameters_[i].type != ParameterType::##TYPE_NAME)                                     \
        {                                                                                          \
            std::cout << "Parameter type does not match " << i << std::endl;                       \
            return DEFAULT_VALUE;                                                                  \
        }                                                                                          \
        return parameters_[i].##UNION_ELEMENT##_value;                                             \
    }

constexpr glm::vec4 DEFAULT_VEC4 = glm::vec4{0, 0, 0, 1};
constexpr glm::mat4 DEFAULT_MAT4 = glm::mat4{1.0f};

DEFINE_PARAMTERS_METHODS(Float, float, float, float, {});
DEFINE_PARAMTERS_METHODS(Vec2, glm::vec2, glm::vec2, vec2, {});
DEFINE_PARAMTERS_METHODS(Vec3, glm::vec3, glm::vec3, vec3, {});
DEFINE_PARAMTERS_METHODS(Vec4, glm::vec4, glm::vec4, vec4, DEFAULT_VEC4);
DEFINE_PARAMTERS_METHODS(Mat4, glm::mat4, const glm::mat4 &, mat4, DEFAULT_MAT4)

const char *Material::getParameterTypeName(ParameterType type)
{
    switch (type)
    {
    case ParameterType::Float: return "Float";
    case ParameterType::Vec2: return "Vec2";
    case ParameterType::Vec3: return "Vec3";
    case ParameterType::Vec4: return "Vec4";
    case ParameterType::Mat4: return "Mat4";
    default: return "Unknown";
    }
}

Material::Material() = default;

Material::~Material() = default;

Material::Material(Material &&other) noexcept
{
    *this = std::move(other);
}

Material &Material::operator=(Material &&other) noexcept
{
    if (this != &other)
    {
        ShaderSource *source = getShaderSource();

        shader_.setSource(source);
        shader_.recompile();
        parameters_ = std::move(other.parameters_);
        textures_ = std::move(other.textures_);
        defines_ = std::move(other.defines_);

        other.shader_.clearAll();
        other.two_sided_ = false;
    }
    return *this;
}

Material Material::clone() const
{
    Material material;
    material.shader_.setSource(shader_.getSource());
    material.shader_.recompile();
    material.parameters_ = parameters_;
    material.textures_ = textures_;
    material.defines_ = defines_;
    material.two_sided_ = two_sided_;
    return material;
}

ShaderSource *Material::getShaderSource() const
{
    return shader_.getSource();
}

void Material::setShaderSource(ShaderSource *source)
{
    shader_.setSource(source);
}

void Material::clearShaderSource()
{
    shader_.setSource(nullptr);
}

Shader *Material::getShader()
{
    return &shader_;
}

bool Material::hasParameter(const char *name) const
{
    return find_parameter(name) != -1;
}

Material::ParameterType Material::getParameterType(int i) const
{
    return parameters_[i].type;
}

const char *Material::getParameterTypeName(int i) const
{
    return getParameterTypeName(getParameterType(i));
}

int Material::getNumParameters() const
{
    return parameters_.size();
}

void Material::clearParameters()
{
    parameters_.clear();
}

int Material::addTexture(const char *name)
{
    return addTexture(name, nullptr);
}

int Material::addTexture(const char *name, Texture *texture)
{
    const int index = find_texture(name);
    if (index != -1)
    {
        std::cout << "Texture " << name << " already exists" << std::endl;
        return -1;
    }
    TextureInfo texture_info;
    texture_info.name = name;
    texture_info.texture = texture;
    const int i = textures_.size();
    textures_.push_back(std::move(texture_info));
    return i;
}

void Material::setTexture(const char *name, Texture *texture)
{
    const int index = find_texture(name);
    if (index == -1)
    {
        std::cout << "Texture " << name << " not found" << std::endl;
        return;
    }
    textures_[index].texture = texture;
}

void Material::setTexture(int i, Texture *texture)
{
    textures_[i].texture = texture;
}

int Material::getNumTextures() const
{
    return textures_.size();
}

void Material::clearTextures()
{
    textures_.clear();
}

int Material::addDefine(const char *name)
{
    return addDefine(name, false);
}

int Material::addDefine(const char *name, bool enabled)
{
    const int index = find_define(name);
    if (index != -1)
    {
        std::cout << "Define " << name << " already exists" << std::endl;
        return -1;
    }
    Define define;
    define.name = name;
    define.enabled = enabled;
    const int i = defines_.size();
    defines_.push_back(std::move(define));
    return i;
}

void Material::setDefine(int i, bool enabled)
{
    defines_[i].enabled = enabled;
}

void Material::setDefine(const char *name, bool enabled)
{
    const int index = find_define(name);
    if (index == -1)
    {
        std::cout << "Define " << name << " not found" << std::endl;
        return;
    }
    defines_[index].enabled = enabled;
}

bool Material::getDefine(int i) const
{
    return defines_[i].enabled;
}

bool Material::getDefine(const char *name) const
{
    const int index = find_define(name);
    if (index == -1)
    {
        std::cout << "Define " << name << " not found" << std::endl;
        return false;
    }
    return defines_[index].enabled;
}

int Material::getNumDefines() const
{
    return defines_.size();
}

void Material::clearDefines()
{
    defines_.clear();
}

int Material::find_parameter(const char *name) const
{
    for (int i = 0; i < parameters_.size(); i++)
    {
        if (parameters_[i].name == name)
        {
            return i;
        }
    }
    return -1;
}

int Material::find_texture(const char *name) const
{
    for (int i = 0; i < textures_.size(); i++)
    {
        if (textures_[i].name == name)
        {
            return i;
        }
    }
    return -1;
}

int Material::find_define(const char *name) const
{
    for (int i = 0; i < defines_.size(); i++)
    {
        if (defines_[i].name == name)
        {
            return i;
        }
    }
    return -1;
}
