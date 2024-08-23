#include "Shader.h"

#include "EngineGlobals.h"
#include "fs/FileSystem.h"
#include "glad/glad.h"

#include "glm/gtc/type_ptr.inl"
#include "glm/mat4x4.hpp"
#include <cassert>
#include <iostream>

Shader::Shader() = default;

Shader::Shader(const char *vertex_src, const char *fragment_src)
{
    loadSources(vertex_src, fragment_src);
}

Shader::Shader(const char *path)
{
    loadFile(path);
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

void Shader::setUniformFloat(const char *name, float value)
{
    const int location = get_uniform_location(name);
    if (location != -1)
    {
        glUniform1f(location, value);
    }
}

void Shader::setUniformVec3(const char *name, const glm::vec3 &value)
{
    const int location = get_uniform_location(name);
    if (location != -1)
    {
        glUniform3f(location, value.x, value.y, value.z);
    }
}

void Shader::setUniformVec4(const char *name, const glm::vec4 &value)
{
    const int location = get_uniform_location(name);
    if (location != -1)
    {
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }
}

void Shader::setUniformMat4(const char *name, const glm::mat4 &value)
{
    const int location = get_uniform_location(name);
    if (location != -1)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }
}

void Shader::setDefine(const char *name)
{
    defines_.insert(name);
}

void Shader::clearDefine(const char *name)
{
    defines_.erase(name);
}

void Shader::recompile()
{
    clearProgram();
    assert(program_id_ == 0);
    if (!filepath_.empty())
    {
        assert(vertex_src_.empty() && fragment_src_.empty());
        std::string vertex_source;
        std::string fragment_source;
        read_shader(filepath_.c_str(), defines_, vertex_source, fragment_source);
        program_id_ = compile_shader(vertex_source.c_str(), fragment_source.c_str());
    }
    else
    {
        assert(!vertex_src_.empty() && !fragment_src_.empty());
        program_id_ = compile_shader(vertex_src_.c_str(), fragment_src_.c_str());
    }

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

void Shader::read_shader(const char *path, const std::unordered_set<std::string> &defines,
    std::string &vertex, std::string &fragment)
{
    vertex.clear();
    fragment.clear();

    std::string shaders = engine_globals.fs->readFile(path);
    apply_defines(shaders, defines);

    const int vertex_idx = shaders.find("#vertex");
    const int fragment_idx = shaders.find("#fragment");
    const int vertex_idx_end = vertex_idx + strlen("#vertex");
    const int fragment_idx_end = fragment_idx + strlen("#fragment");

    assert(vertex_idx < fragment_idx);

    std::string common = shaders.substr(0, vertex_idx);

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
    vertex += shaders.substr(vertex_idx_end, fragment_idx - vertex_idx_end);

    fragment = std::move(common);
    replace_with(fragment, "#inout", "in");
    fragment += shaders.substr(fragment_idx_end);
}

void Shader::apply_defines(std::string &shader, const std::unordered_set<std::string> &defines)
{
    const auto is_part_of_name = [](const char ch) {
        return isdigit(ch) || isalpha(ch) || ch == '_';
    };

    // doesn't move pointer if does not match or moves the pointer after the expression
    const auto check_ifdef_str = [&](char *name, std::string &define_name,
                                     bool &matched) -> char * {
        if (strncmp(name, "#ifdef ", 7) == 0)
        {
            auto name_begin = name + 7;
            auto name_end = name_begin;
            while (is_part_of_name(*name_end))
            {
                name_end++;
            }
            define_name = std::string(name_begin, name_end);
            matched = true;
            return name_end;
        }
        matched = false;
        return name;
    };

    const auto check_ifndef_str = [&](char *name, std::string &define_name,
                                 bool &matched) -> char * {
        if (strncmp(name, "#ifndef ", 8) == 0)
        {
            auto name_begin = name + 8;
            auto name_end = name_begin;
            while (is_part_of_name(*name_end))
            {
                name_end++;
            }
            define_name = std::string(name_begin, name_end);
            matched = true;
            return name_end;
        }
        matched = false;
        return name;
    };

    const auto check_endif_str = [&](char *name, bool &matched) -> char * {
        if (strncmp(name, "#endif", 6) == 0)
        {
            matched = true;
            return name + 6;
        }
        matched = false;
        return name;
    };

    std::vector<bool> state_stack;
    bool is_compiled = true;

    std::string cur_define_name; // tmp
    bool matched = false;        // tmp

    std::string out;

    char *it = shader.data();
    char *end = it + shader.size();
    while (it < end)
    {
        it = check_ifdef_str(it, cur_define_name, matched);
        if (matched)
        {
            if (cur_define_name.empty())
            {
                std::cout << "#ifdef name is empty" << std::endl;
                return;
            }
            state_stack.push_back(is_compiled);
            const bool defined = defines.find(cur_define_name) != defines.end();
            is_compiled &= defined;
            continue;
        }

        it = check_ifndef_str(it, cur_define_name, matched);
        if (matched)
        {
            if (cur_define_name.empty())
            {
                std::cout << "#ifndef name is empty" << std::endl;
                return;
            }
            state_stack.push_back(is_compiled);
            const bool defined = defines.find(cur_define_name) == defines.end();
            is_compiled &= defined;
            continue;
        }

        it = check_endif_str(it, matched);
        if (matched)
        {
            if (state_stack.empty())
            {
                std::cout << "Too many #endif" << std::endl;
                return;
            }
            is_compiled = state_stack.back();
            state_stack.pop_back();
            continue;
        }

        if (is_compiled)
        {
            out += *it;
        }
        it++;
    }
    if (!state_stack.empty())
    {
        std::cout << "Too many #ifdef" << std::endl;
    }
    shader = std::move(out);
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

int Shader::get_uniform_location(const char *name)
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
    if (location == -1)
    {
        std::cout << "Could not find uniform: " << name << std::endl;
    }
    else
    {
        uniform_locations_[name] = location;
    }
    return location;
}