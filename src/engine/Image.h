#pragma once

class Image
{
    explicit Image(const char *path, bool flip_y = true);
    ~Image();

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    int getNumChannels() const { return num_ch_; }

    unsigned char *getData() { return data_; }

    void clear();

private:
    unsigned char *data_{};
    int width_{-1};
    int height_{-1};
    int num_ch_{-1};
};
