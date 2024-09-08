#pragma once

#include "Base.h"

#include <string>
#include <vector>

class Shader;

class ShaderSource
{
public:
    REMOVE_COPY_MOVE_CLASS(ShaderSource);

    ShaderSource();
    ~ShaderSource();

    ShaderSource(const char *vertex_src, const char *fragment_src);
    explicit ShaderSource(const char *path);

    const std::string &getFilepath();

    void setSources(const char *vertex_src, const char *fragment_src);
    void setFile(const char *path);

    std::string makeSourceVertex(const std::vector<std::string> &defines) const;
    std::string makeSourceFragment(const std::vector<std::string> &defines) const;

    void refresh();

    bool isFromFile() const { return is_from_file_; }

private:
    void notify_changed();
    void load_sources_from_file();

    static std::string make_shader(const std::string &shader,
        const std::vector<std::string> &defines);
    static void read_from_file(const char *path, std::string &vertex, std::string &fragment);

    // TODO: shitty?
    friend Shader;
    void add_shader(Shader *shader);
    void remove_shader(Shader *shader);

private:
    std::vector<Shader *> shaders_;

    bool is_from_file_{false};
    std::string filepath_;
    std::string vertex_src_;
    std::string fragment_src_;
};
