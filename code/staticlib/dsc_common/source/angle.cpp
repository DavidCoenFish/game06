#include "angle.h"
#include "math.h"

const float Angle::DegToRadian(const float in_deg)
{
	return in_deg * DSC_PI_DIV_180;
}

const float Angle::RadianToDeg(const float in_radian)
{
	return in_radian * DSC_180_DIV_PI;
}


