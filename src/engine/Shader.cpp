#include "Shader.h"

#include "EngineGlobals.h"
#include "fs/FileSystem.h"
#include "glad/glad.h"

#include "glm/gtc/type_ptr.inl"
#include "glm/mat4x4.hpp"
#include <cassert>
#include <iostream>


namespace
{

const char SHADER_VERSION_LINE[] = "#version 330 core\n";

}

Shader::Shader() = default;

Shader::Shader(const char *vertex_src, const char *fragment_src)
{
    loadSources(vertex_src, fragment_src);
}

Shader::Shader(const char *path)
{
    loadFile(path);
}

Shader::Shader(Shader &&other) noexcept
{
    *this = std::move(other);
}

Shader &Shader::operator=(Shader &&other) noexcept
{
    if (this != &other)
    {
        clearAll();
        filepath_ = std::move(other.filepath_);
        vertex_src_ = std::move(other.vertex_src_);
        fragment_src_ = std::move(other.fragment_src_);
        uniform_locations_ = std::move(other.uniform_locations_);
        program_id_ = other.program_id_;
        defines_ = std::move(other.defines_);
        compiled_defines_ = std::move(other.compiled_defines_);
        other.program_id_ = 0;
    }
    return *this;
}


Shader::~Shader()
{
    clearAll();
}

void Shader::loadSources(const char *vertex_src, const char *fragment_src)
{
    filepath_.clear();
    vertex_src_ = vertex_src;
    fragment_src_ = fragment_src;
    recompile();
}

void Shader::loadFile(const char *path)
{
    filepath_ = path;
    vertex_src_.clear();
    fragment_src_.clear();
    recompile();
}

void Shader::setUniformFloat(int location, float value)
{
    assert(location != -1);
    glUniform1f(location, value);
}

void Shader::setUniformVec2(int location, const glm::vec2 &value)
{
    assert(location != -1);
    glUniform2f(location, value.x, value.y);
}

void Shader::setUniformVec3(int location, const glm::vec3 &value)
{
    assert(location != -1);
    glUniform3f(location, value.x, value.y, value.z);
}

void Shader::setUniformVec4(int location, const glm::vec4 &value)
{
    assert(location != -1);
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Shader::setUniformMat4(int location, const glm::mat4 &value)
{
    assert(location != -1);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setUniformInt(int location, int value)
{
    assert(location != -1);
    glUniform1i(location, value);
}

void Shader::setUniformFloat(const char *name, float value)
{
    const int location = get_uniform_location_with_warning(name);
    if (location != -1)
    {
        setUniformFloat(location, value);
    }
}

void Shader::setUniformVec2(const char *name, const glm::vec2 &value)
{
    const int location = get_uniform_location_with_warning(name);
    if (location != -1)
    {
        setUniformVec2(location, value);
    }
}

void Shader::setUniformVec3(const char *name, const glm::vec3 &value)
{
    const int location = get_uniform_location_with_warning(name);
    if (location != -1)
    {
        setUniformVec3(location, value);
    }
}

void Shader::setUniformVec4(const char *name, const glm::vec4 &value)
{
    const int location = get_uniform_location_with_warning(name);
    if (location != -1)
    {
        setUniformVec4(location, value);
    }
}

void Shader::setUniformMat4(const char *name, const glm::mat4 &value)
{
    const int location = get_uniform_location_with_warning(name);
    if (location != -1)
    {
        setUniformMat4(location, value);
    }
}

void Shader::setUniformInt(const char *name, int value)
{
    const int location = get_uniform_location_with_warning(name);
    if (location != -1)
    {
        setUniformInt(location, value);
    }
}

void Shader::setDefine(const char *name, bool value)
{
    if (value)
    {
        addDefine(name);
    }
    else
    {
        removeDefine(name);
    }
}

void Shader::addDefine(const char *name)
{
    defines_.insert(name);
}

void Shader::removeDefine(const char *name)
{
    defines_.erase(name);
}

void Shader::clearDefines()
{
    defines_.clear();
}

void Shader::recompile()
{
    clearProgram();
    assert(program_id_ == 0);

    std::string vertex_source;
    std::string fragment_source;

    if (!filepath_.empty())
    {
        assert(vertex_src_.empty() && fragment_src_.empty());
        read_from_file(filepath_.c_str(), vertex_source, fragment_source);
    }
    else
    {
        assert(!vertex_src_.empty() && !fragment_src_.empty());
        vertex_source = vertex_src_;
        fragment_source = fragment_src_;
    }

    prepare_shader(vertex_source, defines_);
    prepare_shader(fragment_source, defines_);
    program_id_ = compile_shader(vertex_source.c_str(), fragment_source.c_str());

    if (program_id_ != 0)
    {
        compiled_defines_ = defines_;
    }
    else
    {
        compiled_defines_.clear();
    }
}

bool Shader::isLoaded() const
{
    return program_id_ != 0;
}

void Shader::clearProgram()
{
    if (program_id_ != 0)
    {
        glDeleteProgram(program_id_);
        program_id_ = 0;
    }
    uniform_locations_.clear();
}

void Shader::clearAll()
{
    filepath_.clear();
    vertex_src_.clear();
    fragment_src_.clear();
    defines_.clear();
    compiled_defines_.clear();
    clearProgram();
}

void Shader::bind()
{
    if (program_id_ == 0)
    {
        std::cout << "Shader is not loaded\n" << std::endl;
    }
    glUseProgram(program_id_);
}

bool Shader::isDirty() const
{
    return !isLoaded() || defines_ != compiled_defines_;
}

int Shader::getUniformLocation(const char *name)
{
    if (!isLoaded())
    {
        return -1;
    }
    auto it = uniform_locations_.find(name);
    if (it != uniform_locations_.end())
    {
        assert(it->second == glGetUniformLocation(program_id_, name));
        return it->second;
    }
    const int location = glGetUniformLocation(program_id_, name);
    uniform_locations_[name] = location;
    return location;
}

int Shader::hasUniform(const char *name)
{
    return getUniformLocation(name) != -1;
}

unsigned int Shader::compile_shader(const char *vertex_src, const char *fragment_src)
{
    unsigned int vertex_id;
    vertex_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_id, 1, &vertex_src, NULL);
    glCompileShader(vertex_id);
    bool success = check_compiler_errors(vertex_id, "VERTEX");
    if (!success)
    {
        glDeleteShader(vertex_id);
        return 0;
    }

    unsigned int fragment_id;
    fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_id, 1, &fragment_src, NULL);
    glCompileShader(fragment_id);
    success = check_compiler_errors(fragment_id, "FRAGMENT");
    if (!success)
    {
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
        return 0;
    }

    unsigned int program_id = glCreateProgram();
    glAttachShader(program_id, vertex_id);
    glAttachShader(program_id, fragment_id);
    glLinkProgram(program_id);
    success = check_linking_errors(program_id);
    if (!success)
    {
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
        glDeleteProgram(program_id);
        return 0;
    }

    glUseProgram(program_id);
    glDeleteShader(vertex_id);
    glDeleteShader(fragment_id);
    return program_id;
}

void Shader::read_from_file(const char *path, std::string &vertex, std::string &fragment)
{
    vertex.clear();
    fragment.clear();

    std::string shaders_source = eg.fs->readFile(path);

    const int vertex_idx = shaders_source.find("#vertex");
    const int vertex_idx_end = vertex_idx + strlen("#vertex");

    const int fragment_idx = shaders_source.find("#fragment");
    const int fragment_idx_end = fragment_idx + strlen("#fragment");

    assert(vertex_idx < fragment_idx);

    std::string common = shaders_source.substr(0, vertex_idx);

    const auto replace_with = [](std::string &string, const char *from, const char *to) {
        std::string::size_type pos = 0;
        while ((pos = string.find(from, pos)) != std::string::npos)
        {
            string.replace(pos, strlen(from), to);
            pos += strlen(to);
        }
    };

    vertex = common;
    replace_with(vertex, "#inout", "out");
    vertex += shaders_source.substr(vertex_idx_end, fragment_idx - vertex_idx_end);

    fragment = std::move(common);
    replace_with(fragment, "#inout", "in");
    fragment += shaders_source.substr(fragment_idx_end);
}

void Shader::prepare_shader(std::string &shader, const std::unordered_set<std::string> &defines)
{
    std::string header;
    header += SHADER_VERSION_LINE;

    for (const std::string &define : defines)
    {
        header += "#define " + define + "\n";
    }

    shader = header + shader;
}

bool Shader::check_compiler_errors(unsigned int shader, const char *type)
{
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED (" << type << "):\n"
                  << infoLog << std::endl;
    }
    return success;
}

bool Shader::check_linking_errors(unsigned int program)
{
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::LINKING_FAILED:\n" << infoLog << std::endl;
    }
    return success;
}

int Shader::get_uniform_location_with_warning(const char *name)
{
    const int location = getUniformLocation(name);
    if (location == -1)
    {
        std::cout << "Could not find uniform: " << name << std::endl;
    }
    return location;
}