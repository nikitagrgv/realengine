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
        assert(isBase());                                                                          \
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
        assert(isBase() || isParameterOverriden(name));                                            \
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
        assert(isBase() || isParameterOverriden(name));                                            \
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

Material::Material(Material *parent)
{
    parent_mat_ = parent;
    base_mat_ = parent ? parent->base_mat_ : this;

    if (!parent)
    {
        shader_ = makeU<Shader>();
    }
    else
    {
        make_child_parent(this, parent);
        inherited_.defines.resize(parent->getNumDefines());
        inherited_.parameters.resize(parent->getNumParameters());
        inherited_.textures.resize(parent->getNumTextures());
    }
}

Material::~Material()
{
    assert(children_.empty());
    if (parent_mat_)
    {
        unmake_child_parent(this, parent_mat_);
    }
}

UPtr<Material> Material::clone() const
{
    UPtr<Material> cloned = makeU<Material>(parent_mat_);

    // TODO# implement
    std::cout << "NOT IMPLEMENTD" << std::endl;

    return std::move(cloned);
}

UPtr<Material> Material::inherit() const
{
    UPtr<Material> inherited = makeU<Material>(this);
    return std::move(inherited);
}

ShaderSource *Material::getShaderSource() const
{
    return base_mat_->shader_->getSource();
}

void Material::setShaderSource(ShaderSource *source)
{
    assert(isBase() && children_.empty());
    shader_->setSource(source);
}

void Material::clearShaderSource()
{
    assert(isBase() && children_.empty());
    shader_->setSource(nullptr);
}

Shader *Material::getShader()
{
    Material *cur = this;
    while (cur)
    {
        if (cur->shader_)
        {
            return cur->shader_.get();
        }
        cur = cur->parent_mat_;
    }
    assert(0);
    return nullptr;
}

bool Material::hasParameter(const char *name) const
{
    return find_parameter(name) != -1;
}

Material::ParameterType Material::getParameterType(int i) const
{
    return base_mat_->base_.parameters[i].type;
}

const char *Material::getParameterTypeName(int i) const
{
    return getParameterTypeName(getParameterType(i));
}

const std::string &Material::getParameterName(int i) const
{
    return base_mat_->base_.parameters[i].name;
}

int Material::getNumParameters() const
{
    return base_mat_->base_.parameters.size();
}

void Material::clearParameters()
{
    assert(isBase() && children_.empty());
    base_.parameters.clear();
}

void Material::setParameterOverriden(const char *name, bool overriden)
{
    assert(!isBase());
    const int index = find_parameter(name);
    if (index == -1)
    {
        std::cout << "Parameter " << name << " not found" << std::endl;
        return;
    }
    setParameterOverriden(index, overriden);
}

void Material::setParameterOverriden(int i, bool overriden)
{
    assert(!isBase());
    ParameterOverride &ov = inherited_.parameters[i];
    if (ov.override == overriden)
    {
        return;
    }
    ov.override = overriden;
    if (overriden)
    {
        static_assert((int)ParameterType::NUM_TYPES == 5);
        switch (getParameterType(i))
        {
        case ParameterType::Float:
        {
            ov.float_value = getParameterFloat(i);
            break;
        }
        case ParameterType::Vec2:
        {
            ov.vec2_value = getParameterVec2(i);
            break;
        }
        case ParameterType::Vec3:
        {
            ov.vec3_value = getParameterVec3(i);
            break;
        }
        case ParameterType::Vec4:
        {
            ov.vec4_value = getParameterVec4(i);
            break;
        }
        case ParameterType::Mat4:
        {
            ov.mat4_value = getParameterMat4(i);
            break;
        }
        default: assert(0); break;
        }
    }
}

bool Material::isParameterOverriden(const char *name) const
{
    assert(!isBase());
    const int index = find_parameter(name);
    if (index == -1)
    {
        std::cout << "Parameter " << name << " not found" << std::endl;
        return false;
    }
    return isParameterOverriden(index);
}

bool Material::isParameterOverriden(int i) const
{
    assert(!isBase());
    return inherited_.parameters[i].override;
}

int Material::addTexture(const char *name)
{
    return addTexture(name, nullptr);
}

int Material::addTexture(const char *name, Texture *texture)
{
    assert(isBase() && children_.empty());
    if (find_texture(name) != -1)
    {
        std::cout << "Texture " << name << " already exists" << std::endl;
        return -1;
    }

    TextureInfo ti;
    ti.name = name;
    ti.texture = texture;
    const int index = base_.textures.size();
    base_.textures.push_back(std::move(ti));
    return index;
}

void Material::setTexture(const char *name, Texture *texture)
{
    const int index = find_texture(name);
    if (index == -1)
    {
        std::cout << "Texture " << name << " not found" << std::endl;
        return;
    }
    setTexture(index, texture);
}

void Material::setTexture(int i, Texture *texture)
{
    assert(isBase() || isTextureOverriden(i));
    Texture *&value = isBase() ? base_.textures[i].texture : inherited_.textures[i].texture;
    if (value == texture)
    {
        return;
    }
    value = texture;
}

Texture *Material::getTexture(int i) const
{
    const Material *cur = this;
    while (!cur->isBase())
    {
        const TextureInfoOverride ov = cur->inherited_.textures[i];
        if (ov.override)
        {
            return ov.texture;
        }
        cur = cur->parent_mat_;
    }
    assert(cur == base_mat_);
    return cur->base_.textures[i].texture;
}

const std::string &Material::getTextureName(int i) const
{
    return base_mat_->base_.textures[i].name;
}

int Material::getNumTextures() const
{
    return base_mat_->base_.textures.size();
}

void Material::clearTextures()
{
    assert(isBase() && children_.empty());
    base_.textures.clear();
}

void Material::setTextureOverriden(const char *name, bool overriden)
{
    assert(!isBase());
    const int index = find_texture(name);
    if (index == -1)
    {
        std::cout << "Texture " << name << " not found" << std::endl;
        return;
    }
    setTextureOverriden(index, overriden);
}

void Material::setTextureOverriden(int i, bool overriden)
{
    assert(!isBase());
    TextureInfoOverride &ov = inherited_.textures[i];
    if (ov.override == overriden)
    {
        return;
    }
    ov.override = overriden;
    if (overriden)
    {
        ov.texture = parent_mat_->getTexture(i);
    }
}

bool Material::isTextureOverriden(const char *name) const
{
    assert(!isBase());
    const int index = find_texture(name);
    if (index == -1)
    {
        std::cout << "Texture " << name << " not found" << std::endl;
        return false;
    }
    return isTextureOverriden(index);
}

bool Material::isTextureOverriden(int i) const
{
    assert(!isBase());
    return inherited_.textures[i].override;
}

int Material::addDefine(const char *name)
{
    return addDefine(name, false);
}

int Material::addDefine(const char *name, bool enabled)
{
    assert(isBase() && children_.empty());
    if (find_define(name) != -1)
    {
        std::cout << "Define " << name << " already exists" << std::endl;
        return -1;
    }

    Define define;
    define.name = name;
    define.enabled = enabled;
    const int index = base_.defines.size();
    base_.defines.push_back(std::move(define));
    if (enabled)
    {
        set_defines_to_shader();
    }
    return index;
}

void Material::setDefine(int i, bool enabled)
{
    assert(isBase() || isDefineOverriden(i));
    bool &value = isBase() ? base_.defines[i].enabled : inherited_.defines[i].enabled;
    if (value == enabled)
    {
        return;
    }
    value = enabled;
    set_defines_to_shader();
}

void Material::setDefine(const char *name, bool enabled)
{
    const int index = find_define(name);
    if (index == -1)
    {
        std::cout << "Define " << name << " not found" << std::endl;
        return;
    }
    setDefine(index, enabled);
}

const std::string &Material::getDefineName(int i) const
{
    return base_mat_->base_.defines[i].name;
}

bool Material::getDefine(int i) const
{
    const Material *cur = this;
    while (!cur->isBase())
    {
        const DefineOverride ov = cur->inherited_.defines[i];
        if (ov.override)
        {
            return ov.enabled;
        }
        cur = cur->parent_mat_;
    }
    assert(cur == base_mat_);
    return cur->base_.defines[i].enabled;
}

bool Material::getDefine(const char *name) const
{
    const int index = find_define(name);
    if (index == -1)
    {
        std::cout << "Define " << name << " not found" << std::endl;
        return false;
    }
    return getDefine(index);
}

int Material::getNumDefines() const
{
    return base_mat_->base_.defines.size();
}

void Material::clearDefines()
{
    assert(isBase() && children_.empty());
    base_.defines.clear();
    set_defines_to_shader();
}

void Material::setDefineOverriden(const char *name, bool overriden)
{
    assert(!isBase());
    const int index = find_define(name);
    if (index == -1)
    {
        std::cout << "Define " << name << " not found" << std::endl;
        return;
    }
    setDefineOverriden(index, overriden);
}

void Material::setDefineOverriden(int i, bool overriden)
{
    assert(!isBase());
    DefineOverride &ov = inherited_.defines[i];
    if (ov.override == overriden)
    {
        return;
    }
    ov.override = overriden;
    if (overriden)
    {
        ov.enabled = parent_mat_->getDefine(i);
    }
    on_define_overrides_changed();
}

bool Material::isDefineOverriden(const char *name) const
{
    assert(!isBase());
    const int index = find_define(name);
    if (index == -1)
    {
        std::cout << "Define " << name << " not found" << std::endl;
        return false;
    }
    return isDefineOverriden(index);
}

bool Material::isDefineOverriden(int i) const
{
    assert(!isBase());
    return inherited_.defines[i].override;
}

// TODO: macros?
bool Material::isTwoSided() const
{
    const Material *cur = this;
    while (!cur->isBase())
    {
        if (cur->options_.two_sided.override)
        {
            return cur->options_.two_sided.value;
        }
        cur = cur->parent_mat_;
    }
    assert(cur == base_mat_);
    return cur->options_.two_sided.value;
}

void Material::setTwoSided(bool two_sided)
{
    assert(isBase() || isTwoSidedOverriden());
    options_.two_sided.value = two_sided;
}

void Material::setTwoSidedOverriden(bool overriden)
{
    assert(!isBase());
    if (options_.two_sided.override == overriden)
    {
        return;
    }
    options_.two_sided.override = overriden;
    if (overriden)
    {
        options_.two_sided.value = parent_mat_->isTwoSided();
    }
}

bool Material::isTwoSidedOverriden() const
{
    assert(!isBase());
    return options_.two_sided.override;
}

void Material::set_defines_to_shader()
{
    std::vector<std::string> defines;
    for (int i = 0; i < getNumDefines(); ++i)
    {
        if (getDefine(i))
        {
            defines.push_back(getDefineName(i));
        }
    }
    shader_->setDefines(std::move(defines));
}

void Material::on_define_overrides_changed()
{
    bool has_any_overrides = false;
    for (const DefineOverride &define : inherited_.defines)
    {
        if (define.override)
        {
            has_any_overrides = true;
            break;
        }
    }

    if (has_any_overrides)
    {
        if (!shader_)
        {
            shader_ = makeU<Shader>(getShaderSource());
        }
        set_defines_to_shader();
    }
    else
    {
        if (shader_)
        {
            shader_.reset();
        }
    }
}

int Material::find_parameter(const char *name) const
{
    const std::vector<Parameter> &parameters = base_mat_->base_.parameters;
    for (int i = 0, count = parameters.size(); i < count; i++)
    {
        if (parameters[i].name == name)
        {
            return i;
        }
    }
    return -1;
}

int Material::find_texture(const char *name) const
{
    const std::vector<TextureInfo> &textures = base_mat_->base_.textures;
    for (int i = 0, count = textures.size(); i < count; i++)
    {
        if (textures[i].name == name)
        {
            return i;
        }
    }
    return -1;
}

int Material::find_define(const char *name) const
{
    const std::vector<Define> &defines = base_mat_->base_.defines;
    for (int i = 0, count = defines.size(); i < count; i++)
    {
        if (defines[i].name == name)
        {
            return i;
        }
    }
    return -1;
}

void Material::make_child_parent(Material *child, Material *parent)
{
#ifndef NDEBUG
    for (auto c : parent->children_)
    {
        assert(c != child);
    }
#endif

    child->parent_mat_ = parent;
    parent->children_.push_back(child);
}

void Material::unmake_child_parent(Material *child, Material *parent)
{
    assert(std::count(parent->children_.begin(), parent->children_.end(), child) == 1);
    assert(child->parent_mat_ == parent);
    for (int i = 0; i < parent->children_.size(); ++i)
    {
        Material *c = parent->children_[i];
        if (c == child)
        {
            // TODO: shitty
            parent->children_.erase(parent->children_.begin() + i);
            child->parent_mat_ = nullptr;
            return;
        }
    }
    assert(0);
}
