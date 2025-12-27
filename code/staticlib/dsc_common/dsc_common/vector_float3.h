#pragma once
#include "dsc_common.h"
#include "vector_3.h"

namespace DscCommon
{
	typedef Vector3<float> VectorFloat3;

	const float Length(const VectorFloat3& in_vector);

	const VectorFloat3 Cross(const VectorFloat3& in_lhs, const VectorFloat3& in_rhs);

	// Try to make it obvious that we Return the Normalised value, we don't modify the input
	const VectorFloat3 ReturnNormal(const VectorFloat3& in_vector);

	const VectorFloat3 RotateVectorAroundAxis(const VectorFloat3& in_vector, const VectorFloat3& in_axis, const float in_radians);


} //namespace DscCommon
