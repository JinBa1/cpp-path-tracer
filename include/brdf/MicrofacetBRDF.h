#pragma once

#include "brdf/BRDF.h"
#include "brdf/Microfacet.h"
#include "brdf/FireflyReduction.h"


struct MicrofacetBRDF : BRDF
{
	MicrofacetBRDF(Vector3 normal, Vector3 reflectance, double roughness) : normal(normal), reflectance(reflectance), alpha(FireflyReduction::GetRoughness((roughness * roughness)))
	{
	}

	// Evaluate the cosine-weighted brdf
	//
	Vector3 Evaluate(Vector3 l, Vector3 v) const override {
    const Vector3 n = normal;
	const Vector3 h = normalize(l + v, n);

	const double ndotl = saturate(dot(n, l));
	const double ndotv = saturate(dot(n, v));
	const double ndoth = saturate(dot(n, h));
	const double vdoth = saturate(dot(v, h));

	return Microfacet::F(reflectance, vdoth) * Microfacet::V(alpha, ndotl, ndotv) * Microfacet::D(alpha, ndoth) * ndotl;
    }

	// Sample the cosine-weighted brdf for both the incoming light direction and sample weight
	//
	Vector3 Sample(Vector3& l, Vector3 v, Vector3 sample) const override {
        const Vector3 n = normal;
        const Vector3 h = Microfacet::CalculateNormal(v, n, sample, alpha);
        
        l = reflect_pm(-v, h);

        const double ndotl = saturate(dot(n, l));
        const double vdoth = saturate(dot(v, h));

        return Microfacet::F(reflectance, vdoth) * Microfacet::G1(alpha, ndotl);
    }

	// World-space normal
	//
	Vector3 normal = { 0, 0, 1 };

	// Normal reflectance
	//
	Vector3 reflectance = { 1, 1, 1 };

	// Roughness
	//
	double alpha = 0.5;
};