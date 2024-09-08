#include "ShaderSource.h"

#include "EngineGlobals.h"
#include "Shader.h"
#include "fs/FileSystem.h"

namespace
{

const char SHADER_VERSION_LINE[] = "#version 330 core\n";

}

ShaderSource::ShaderSource() = default;

ShaderSource::~ShaderSource()
{
    for (int i = shaders_.size() - 1; i >= 0; --i)
    {
        assert(shaders_[i]->getSource() == this);
        shaders_[i]->unbindSource();
    }
    assert(shaders_.empty());
}

ShaderSource::ShaderSource(const char *vertex_src, const char *fragment_src)
{
    setSources(vertex_src, fragment_src);
}

ShaderSource::ShaderSource(const char *path)
{
    setFile(path);
}

ShaderSource::ShaderSource(ShaderSource &&other) noexcept
{
    *this = std::move(other);
}

ShaderSource &ShaderSource::operator=(ShaderSource &&other) noexcept
{
    if (this != &other)
    {
        // TODO: shitty
        for (int i = shaders_.size() - 1; i >= 0; --i)
        {
            assert(shaders_[i]->getSource() == this);
            shaders_[i]->unbindSource();
        }
        assert(shaders_.empty());

        for (Shader *s : other.shaders_)
        {
            assert(s->source_ == &other);
            other.remove_shader(s);
            add_shader(s);
        }
        shaders_ = std::move(other.shaders_);
    }
    return *this;
}

const std::string &ShaderSource::getFilepath()
{
    return filepath_;
}

void ShaderSource::setSources(const char *vertex_src, const char *fragment_src)
{
    is_from_file_ = false;
    filepath_.clear();

    vertex_src_ = vertex_src;
    fragment_src_ = fragment_src;

    notify_changed();
}

void ShaderSource::setFile(const char *path)
{
    is_from_file_ = true;

    filepath_ = path;

    load_sources_from_file();

    notify_changed();
}

std::string ShaderSource::makeSourceVertex(const std::vector<std::string> &defines) const
{
    if (vertex_src_.empty())
    {
        return {};
    }
    return make_shader(vertex_src_, defines);
}

std::string ShaderSource::makeSourceFragment(const std::vector<std::string> &defines) const
{
    if (fragment_src_.empty())
    {
        return {};
    }
    return make_shader(fragment_src_, defines);
}

void ShaderSource::refresh()
{
    if (!is_from_file_)
    {
        return;
    }

    load_sources_from_file();

    notify_changed();
}

void ShaderSource::notify_changed()
{
    for (Shader *s : shaders_)
    {
        s->recompile();
    }
}

void ShaderSource::load_sources_from_file()
{
    assert(is_from_file_ && !filepath_.empty());
    read_from_file(filepath_.c_str(), vertex_src_, fragment_src_);
}

std::string ShaderSource::make_shader(const std::string &shader,
    const std::vector<std::string> &defines)
{
    std::string header;
    header += SHADER_VERSION_LINE;
    for (const std::string &define : defines)
    {
        header += "#define " + define + "\n";
    }
    return header + shader;
}

void ShaderSource::read_from_file(const char *path, std::string &vertex, std::string &fragment)
{
    // TODO: shitty

    vertex.clear();
    fragment.clear();

    std::string shaders_source = eng.fs->readFile(path);

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

void ShaderSource::add_shader(Shader *shader)
{
#ifndef NDEBUG
    assert(shader->getSource() == this);
    for (const Shader *s : shaders_)
    {
        assert(s->getSource() == this);
        assert(s != shader);
    }
#endif

    shaders_.push_back(shader);
}

void ShaderSource::remove_shader(Shader *shader)
{
#ifndef NDEBUG
    assert(shader->getSource() == this);
    int num = 0;
    for (const Shader *s : shaders_)
    {
        assert(s->getSource() == this);
        if (s == shader)
        {
            ++num;
        }
    }
    assert(num == 1);
#endif

    // TODO: removeFast
    auto it = std::find(shaders_.begin(), shaders_.end(), shader);
    assert(it != shaders_.end());
    shaders_.erase(it);
}
