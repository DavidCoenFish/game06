#include <dsc_common\dsc_common.h>
#include "math_angle.h"
#include "math.h"

const float DscCommon::MathAngle::DegToRadian(const float in_deg)
{
	return in_deg * DSC_PI_DIV_180;
}

const float DscCommon::MathAngle::RadianToDeg(const float in_radian)
{
	return in_radian * DSC_180_DIV_PI;
}

// find the smallest value to get from lhs to rhs
const float DscCommon::MathAngle::SmallestDelta(const float in_radian_lhs, const float in_radian_rhs)
{
#if 0
	// ensure in [-pi ... +pi]
	const float lhs = fmod(in_radian_lhs + DSC_PI, DSC_2_PI) - DSC_PI;
	const float rhs = fmod(in_radian_rhs + DSC_PI, DSC_2_PI) - DSC_PI;

	// same sign?
	if ((lhs < 0.0f) == (rhs < 0.0f))
	{
		return rhs - lhs;
	}

	// if not the same sign, there is a chance the range wraps around for the shortest path, either + or - 2pi for the lhs as the wrap around
	const float diff = rhs - lhs;
	const float alt_lhs = lhs + ((lhs < 0.0f) ? DSC_2_PI : -DSC_2_PI);
	const float alt_diff = rhs - alt_lhs;
	if (abs(alt_diff) < abs(diff))
	{
		return alt_diff;
	}

	return diff;
#else
	const float diff = in_radian_rhs - in_radian_lhs;
	const float diff_mod = DscCommon::Math::PosFMod(diff + DSC_PI, DSC_2_PI) - DSC_PI;
	return diff_mod;
#endif
}
