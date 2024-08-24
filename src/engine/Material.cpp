#include "Material.h"

#include <cassert>

#define DEFINE_PARAMTERS_METHODS(TYPE_NAME, TYPE_VALUE_GET, TYPE_VALUE_SET, UNION_ELEMENT)         \
    void Material::setParameter##TYPE_NAME(const char *name, TYPE_VALUE_SET value)                 \
    {                                                                                              \
        const int index = find_parameter(name);                                                    \
        if (index == -1)                                                                           \
        {                                                                                          \
            Parameter parameter;                                                                   \
            parameter.name = name;                                                                 \
            parameter.type = ParameterType::##TYPE_NAME;                                           \
            parameter.##UNION_ELEMENT##_value = value;                                             \
            parameters_.push_back(parameter);                                                      \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            assert(parameters_[index].type == ParameterType::##TYPE_NAME);                         \
            parameters_[index].##UNION_ELEMENT##_value = value;                                    \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    TYPE_VALUE_GET Material::getParameter##TYPE_NAME(int i)                                        \
    {                                                                                              \
        assert(parameters_[i].type == ParameterType::##TYPE_NAME);                                 \
        return parameters_[i].##UNION_ELEMENT##_value;                                             \
    }

DEFINE_PARAMTERS_METHODS(Float, float, float, float);
DEFINE_PARAMTERS_METHODS(Vec3, glm::vec3, glm::vec3, vec3);
DEFINE_PARAMTERS_METHODS(Vec4, glm::vec4, glm::vec4, vec4);
DEFINE_PARAMTERS_METHODS(Mat4, glm::mat4, const glm::mat4 &, mat4);

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