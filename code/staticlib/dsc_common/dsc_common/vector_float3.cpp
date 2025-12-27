#include "vector_float3.h"

const DscCommon::Vector3<float> DscCommon::Vector3<float>::s_zero(0.0f, 0.0f, 0.0f);

const float DscCommon::Length(const VectorFloat3& in_vector)
{
	const float dot = Dot(in_vector, in_vector);
	const float length = sqrt(dot);
	return length;
}


const DscCommon::VectorFloat3 DscCommon::Cross(const VectorFloat3& in_lhs, const VectorFloat3& in_rhs)
{
	return DscCommon::VectorFloat3(
		(in_lhs.GetY() * in_rhs.GetZ()) - (in_lhs.GetZ() * in_rhs.GetY()),
		(in_lhs.GetZ() * in_rhs.GetX()) - (in_lhs.GetX() * in_rhs.GetZ()),
		(in_lhs.GetX() * in_rhs.GetY()) - (in_lhs.GetY() * in_rhs.GetX())
	);
}

// Try to make it obvious that we Return the Normalised value, we don't modify the input
const DscCommon::VectorFloat3 DscCommon::ReturnNormal(const VectorFloat3& in_vector)
{
	const float length = Length(in_vector);
	if (0.0f == length)
	{
		//ASSERT?
		return DscCommon::VectorFloat3(1.0f, 0.0f, 0.0f);
	}

	return in_vector * (1.0f / length);
}

const DscCommon::VectorFloat3 DscCommon::RotateVectorAroundAxis(const VectorFloat3& in_vector, const VectorFloat3& in_axis, const float in_radians)
{
	const VectorFloat3 paralle_to_k = in_axis * Dot(in_axis, in_vector);
	const VectorFloat3 perpendicular_to_k = in_vector - paralle_to_k;
	const VectorFloat3 k_cross_v = Cross(in_axis, in_vector);

	const VectorFloat3 result = paralle_to_k + (perpendicular_to_k * (float)cos(in_radians)) + (k_cross_v * (float)sin(in_radians));
	return result;
}
