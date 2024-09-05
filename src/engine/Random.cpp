#include "Random.h"

#include <chrono>
#include <random>

namespace
{

std::default_random_engine generator{};

std::uniform_int_distribution<unsigned int> all_uints(0, UINT_MAX);

std::uniform_real_distribution<float> floats_0_to_1(0.0f, 1.0f);
std::uniform_real_distribution<double> doubles_0_to_1(0.0, 1.0);

} // namespace

void Random::init()
{
    generator = std::default_random_engine(
        std::chrono::steady_clock::now().time_since_epoch().count());
}

unsigned int Random::randUint()
{
    return all_uints(generator);
}

int Random::randInt(int min, int max)
{
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}

float Random::randFloat()
{
    return floats_0_to_1(generator);
}

float Random::randFloat(float min, float max)
{
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(generator);
}

double Random::randDouble()
{
    return doubles_0_to_1(generator);
}

double Random::randDouble(double min, double max)
{
    std::uniform_real_distribution<double> distribution(min, max);
    return distribution(generator);
}
