#pragma once

#include <noise/module/modulebase.h>

namespace noise
{

namespace module
{

// Map y -1..+1 to min..max or min..(min+height)
class MapToMinMax final : public Module
{
public:
    MapToMinMax();

    int GetSourceModuleCount() const override { return 1; }

    double GetValue(double x, double y, double z) const override;

    void SetMinAndMax(double min, double max);
    void SetMinAndHeight(double min, double height);

protected:
    double min_;
    double max_;
    double height_;
};

} // namespace module

} // namespace noise
