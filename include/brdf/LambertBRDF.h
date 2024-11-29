#pragma once

#include "brdf/BRDF.h"
#include "brdf/Lambert.h"

struct LambertBRDF : BRDF
{
    LambertBRDF(Vector3 normal, Vector3 albedo) : normal(normal), albedo(albedo)
    {
    }

    // Evaluate the cosine-weighted brdf
    //
    Vector3 Evaluate(Vector3 l, Vector3 v) const override {
        (void)v;

        return Lambert::Evaluate(saturate(dot(normal, l)), albedo);
    }

    // Sample the cosine-weighted brdf for both the incoming light direction and sample weight
    //
    Vector3 Sample(Vector3& l, Vector3 v, Vector3 sample) const override {
        (void)v;

        // Register a rough bounce with the microfacet model for firefly reduction

        FireflyReduction::RegisterBounce(1.0);

        // Calculate the bounce direction

        l = Lambert::CalculateLightDirection(normal, sample);

        // We'll multiply through by the albedo to get the final weight

        return albedo;
    }

    // World-space normal
    //
    Vector3 normal = { 0, 0, 1 };

    // Reflectance
    //
    Vector3 albedo = { 1, 1, 1 };
};