#pragma once
#include "dsc_common.h"

#include <dsc_common\dsc_common.h>

namespace DscCommon
{
	namespace MathAngle
	{
		const float DegToRadian(const float in_deg);
		const float RadianToDeg(const float in_radian);

		// find the smallest value to get from lhs to rhs
		const float SmallestDelta(const float in_radian_lhs, const float in_radian_rhs);

	} //Angle
} //DscCommon