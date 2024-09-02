#include "Material.h"

#include <iostream>

#define DEFINE_PARAMTERS_METHODS(TYPE_NAME, TYPE_VALUE_GET, TYPE_VALUE_SET, UNION_ELEMENT,         \
    DEFAULT_RETURN)                                                                                \
    void Material::addParameter##TYPE_NAME(const char *name)                                       \
    {                                                                                              \
        const int index = find_parameter(name);                                                    \
        if (index != -1)                                                                           \
        {                                                                                          \
            std::cout << "Parameter already exists: " << name << std::endl;                        \
            return;                                                                                \
        }                                                                                          \
        Parameter parameter;                                                                       \
        parameter.name = name;                                                                     \
        parameter.type = ParameterType::##TYPE_NAME;                                               \
        parameters_.push_back(parameter);                                                          \
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
    TYPE_VALUE_GET Material::getParameter##TYPE_NAME(const char *name)                             \
    {                                                                                              \
        const int index = find_parameter(name);                                                    \
        if (index == -1)                                                                           \
        {                                                                                          \
            std::cout << "Parameter not found: " << name << std::endl;                             \
            return DEFAULT_RETURN;                                                                 \
        }                                                                                          \
        return getParameter##TYPE_NAME(index);                                                     \
    }                                                                                              \
                                                                                                   \
    TYPE_VALUE_GET Material::getParameter##TYPE_NAME(int i)                                        \
    {                                                                                              \
        if (parameters_[i].type != ParameterType::##TYPE_NAME)                                     \
        {                                                                                          \
            std::cout << "Parameter type does not match " << i << std::endl;                       \
            return DEFAULT_RETURN;                                                                 \
        }                                                                                          \
        return parameters_[i].##UNION_ELEMENT##_value;                                             \
    }

DEFINE_PARAMTERS_METHODS(Float, float, float, float, {});
DEFINE_PARAMTERS_METHODS(Vec2, glm::vec2, glm::vec2, vec2, {});
DEFINE_PARAMTERS_METHODS(Vec3, glm::vec3, glm::vec3, vec3, {});
DEFINE_PARAMTERS_METHODS(Vec4, glm::vec4, glm::vec4, vec4, {});
DEFINE_PARAMTERS_METHODS(Mat4, glm::mat4, const glm::mat4 &, mat4, glm::mat4{1.0f})

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
        shader_ = other.shader_;
        parameters_ = std::move(other.parameters_);
        textures_ = std::move(other.textures_);

        other.shader_ = nullptr;
    }
    return *this;
}

Material Material::clone() const
{
    Material material;
    material.shader_ = shader_;
    material.parameters_ = parameters_;
    material.textures_ = textures_;
    return material;
}

bool Material::hasParameter(const char *name) const
{
    return find_parameter(name) != -1;
}

Material::ParameterType Material::getParameterType(int i)
{
    return parameters_[i].type;
}

int Material::getNumParameters()
{
    return parameters_.size();
}

void Material::clearParameters()
{
    parameters_.clear();
}

void Material::addTexture(const char *name)
{
    const int index = find_texture(name);
    if (index != -1)
    {
        std::cout << "Texture " << name << " already exists" << std::endl;
    }
    TextureInfo texture_info;
    texture_info.name = name;
    texture_info.texture = nullptr;
    textures_.push_back(texture_info);
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

int Material::getNumTextures()
{
    return textures_.size();
}

void Material::clearTextures()
{
    textures_.clear();
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