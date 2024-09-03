#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class Shader;

class ShaderManager
{
public:
    Shader *createShader(const char *name);

    Shader *addShader(Shader material, const char *name);

    Shader *getShader(const char *name);

    void removeShader(const char *name);
    void removeShader(Shader *material);

private:
    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders_;
    std::unordered_map<Shader *, std::string> shaders_names_;
};