#pragma once

#include <cstdint>

class Time
{
public:
    [[nodiscard]] uint64_t getTimeUsec() const;
    [[nodiscard]] double getTime() const;

    [[nodiscard]] uint64_t getFrame() const { return frame_; }

    [[nodiscard]] double getFrameTime() const { return cur_time_; }
    [[nodiscard]] double getDelta() const { return delta_sec_; }
    [[nodiscard]] double getDeltaMs() const { return delta_ms_; }
    [[nodiscard]] double getFps() const { return fps_; }

private:
    Time();
    ~Time();

    friend class Engine;
    void update();

private:
    uint64_t frame_{0};

    uint64_t cur_time_usec{0};
    uint64_t old_time_usec{0};
    uint64_t start_time_usec{0};

    double cur_time_{};

    double delta_ms_{};
    double delta_sec_{};

    double fps_{};
};
