#include "cc.h"

float factor_frequency(struct edf *edf)
{
    float frequency = 0.00;
    for (int i = 0; i < 100; i++)
    {
        frequency += 0.01;
        if (edf_test(edf, frequency))
            return frequency;
    }
}