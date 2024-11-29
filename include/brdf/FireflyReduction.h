#pragma once

#include "util/Utilities.h"

namespace FireflyReduction
{
    // The minimum allowed roughness for the current sample
    static double min_alpha = 0;

    // The roughness value at which firefly reduction will disable.
    // Roughness values beyond about GGX alpha = 0.25 don't tend to produce fireflies at decent sample rates.
    // We want this to be as low as possible since it will still allow some caustics and generate proper colored reflections.
    static constexpr double max_min_alpha = 0.25f;

    // Prepare for a new sample
    inline void RegisterNewSample()
    {
        min_alpha = 0;
    }

    // Register that a bounce has occurred at the given roughness value
    inline void RegisterBounce(double alpha)
    {
        min_alpha = std::max(min_alpha, std::min(alpha, max_min_alpha));
    }

    // Get the current roughness to use
    inline double GetRoughness(double alpha)
    {
#ifdef FIREFLY_REDUCTION
        return std::max(alpha, min_alpha);
#else
        return alpha;
#endif
    }
}