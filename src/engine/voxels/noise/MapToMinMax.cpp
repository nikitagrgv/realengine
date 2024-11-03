#include "MapToMinMax.h"

using namespace noise::module;

MapToMinMax::MapToMinMax()
    : Module(GetSourceModuleCount())
    , min_(0.0)
    , max_(1.0)
    , height_(1.0)
{}

double MapToMinMax::GetValue(double x, double y, double z) const
{
    assert(m_pSourceModule[0] != NULL);
    const double value = m_pSourceModule[0]->GetValue(x, y, z);
    return (value * 0.5 + 0.5) * height_ + min_;
}

void MapToMinMax::SetMinAndMax(double min, double max)
{
    assert(max > min);
    min_ = min;
    max_ = max;
    height_ = max_ - min_;
}

void MapToMinMax::SetMinAndHeight(double min, double height)
{
    assert(height > 0);
    min_ = min;
    max_ = min + height_;
    height_ = height;
}
