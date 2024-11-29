#pragma once

#include "util/Vector3.h"
#include "brdf/FireflyReduction.h"

struct BRDF
{
	BRDF() = default;

	virtual ~BRDF() = default;

	// Evaluate the cosine-weighted brdf
	//
	virtual Vector3 Evaluate(Vector3 l, Vector3 v) const = 0;

	// Sample the cosine-weighted brdf for both the incoming light direction and sample weight
	//
	virtual Vector3 Sample(Vector3& l, Vector3 v, Vector3 sample) const = 0;
};