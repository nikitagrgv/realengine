#pragma once

class Random
{
public:
    static void init();

    static unsigned int randUint();

    static int randInt(int min, int max);

    static float randFloat(); // 0..1
    static float randFloat(float min, float max);

    static double randDouble(); // 0..1
    static double randDouble(double min, double max);
};
