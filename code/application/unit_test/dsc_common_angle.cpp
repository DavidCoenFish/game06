#include <dsc_common\common.h>
#include <dsc_common\angle.h>
#include "unit_test_util.h"

namespace
{
bool TestValue(const float value0)
{
	const float valueRad0 = DscCommon::Angle::DegToRadian(value0);
	const float valueDeg0 = DscCommon::Angle::RadianToDeg(valueRad0);
	const float valueRad1 = DscCommon::Angle::DegToRadian(valueDeg0);
	
	bool ok = true;
	ok = UNIT_TEST_UTIL_ALMOST_EQUAL(ok, valueRad0, valueRad1);

	return true;
}

};

bool DscCommonAngle()
{
	bool ok = true;
	ok &= TestValue(0.0f);
	ok &= TestValue(0.1f);
	ok &= TestValue(30.0f);
	ok &= TestValue(45.0f);
	ok &= TestValue(90.0f);
	ok &= TestValue(180.0f);
	ok &= TestValue(360.0f);
	ok &= TestValue(720.0f);
	ok &= TestValue(-0.1f);
	ok &= TestValue(-30.0f);
	ok &= TestValue(-45.0f);
	ok &= TestValue(-90.0f);
	ok &= TestValue(-180.0f);
	ok &= TestValue(-360.0f);
	ok &= TestValue(-720.0f);

	ok &= TestValue(123456.789123456f);

	return ok;
}