#include "dsc_common/common.h"
#include "dsc_common/angle.h"
#include "dsc_common/math.h"

const float DscCommon::Angle::DegToRadian(const float in_deg)
{
	return in_deg * DSC_PI_DIV_180;
}

const float DscCommon::Angle::RadianToDeg(const float in_radian)
{
	return in_radian * DSC_180_DIV_PI;
}


