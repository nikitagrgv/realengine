#pragma once
#include "Base.h"

class Image
{
public:
    enum class Format
    {
        INVALID,
        RGBA,
        RGB,
        RG,
        R,
    };

    REMOVE_COPY_MOVE_CLASS(Image);

    Image();
    explicit Image(const char *path, bool flip_y = true);
    ~Image();

    void load(const char *path, bool flip_y = true);

    bool isLoaded() const { return data_ != nullptr; }

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    int getNumChannels() const { return num_ch_; }
    Format getFormat() const { return format_; }

    unsigned char *getData() const { return data_; }

    void clear();

private:
    unsigned char *data_{};
    int width_{-1};
    int height_{-1};
    int num_ch_{-1};
    Format format_{Format::INVALID};
};
