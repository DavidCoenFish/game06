#include "dsc_common_math_angle.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/math_angle.h>
#include "test_util.h"

namespace
{
const bool TestValue(const float value0)
{
	const float valueRad0 = DscCommon::MathAngle::DegToRadian(value0);
	const float valueDeg0 = DscCommon::MathAngle::RadianToDeg(valueRad0);
	const float valueRad1 = DscCommon::MathAngle::DegToRadian(valueDeg0);
	
	bool ok = true;
	ok = TEST_UTIL_ALMOST_EQUAL(ok, valueRad0, valueRad1);

	return ok;
}

const bool TestRoundTripDegRad()
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

const bool TestSmallestAngle()
{
	bool ok = true;

	{
		const float smallest_deg = DscCommon::MathAngle::RadianToDeg(DscCommon::MathAngle::SmallestDelta(
			DscCommon::MathAngle::DegToRadian(5.0f),
			DscCommon::MathAngle::DegToRadian(15.0f)
		));
		ok = TEST_UTIL_ALMOST_EQUAL_EPSILON(ok, smallest_deg, 10.0f, 0.001f);
	}

	{
		const float smallest_deg = DscCommon::MathAngle::RadianToDeg(DscCommon::MathAngle::SmallestDelta(
			DscCommon::MathAngle::DegToRadian(175.0f),
			DscCommon::MathAngle::DegToRadian(185.0f)
		));
		ok = TEST_UTIL_ALMOST_EQUAL_EPSILON(ok, smallest_deg, 10.0f, 0.001f);
	}

	{
		const float smallest_deg = DscCommon::MathAngle::RadianToDeg(DscCommon::MathAngle::SmallestDelta(
			DscCommon::MathAngle::DegToRadian(265.0f),
			DscCommon::MathAngle::DegToRadian(275.0f)
			));
		ok = TEST_UTIL_ALMOST_EQUAL_EPSILON(ok, smallest_deg, 10.0f, 0.001f);
	}

	{
		const float smallest_deg = DscCommon::MathAngle::RadianToDeg(DscCommon::MathAngle::SmallestDelta(
			DscCommon::MathAngle::DegToRadian(275.0f),
			DscCommon::MathAngle::DegToRadian(265.0f)
			));
		ok = TEST_UTIL_ALMOST_EQUAL_EPSILON(ok, smallest_deg, -10.0f, 0.001f);
	}


	{
		const float smallest_deg = DscCommon::MathAngle::RadianToDeg(DscCommon::MathAngle::SmallestDelta(
			DscCommon::MathAngle::DegToRadian(265.0f -360.0f),
			DscCommon::MathAngle::DegToRadian(275.0f)
		));
		ok = TEST_UTIL_ALMOST_EQUAL_EPSILON(ok, smallest_deg, 10.0f, 0.001f);
	}

	{
		const float smallest_deg = DscCommon::MathAngle::RadianToDeg(DscCommon::MathAngle::SmallestDelta(
			DscCommon::MathAngle::DegToRadian(275.0f - 360.0f),
			DscCommon::MathAngle::DegToRadian(265.0f)
		));
		ok = TEST_UTIL_ALMOST_EQUAL_EPSILON(ok, smallest_deg, -10.0f, 0.001f);
	}

	{
		const float smallest_deg = DscCommon::MathAngle::RadianToDeg(DscCommon::MathAngle::SmallestDelta(
			DscCommon::MathAngle::DegToRadian(265.0f),
			DscCommon::MathAngle::DegToRadian(275.0f - 360.0f)
		));
		ok = TEST_UTIL_ALMOST_EQUAL_EPSILON(ok, smallest_deg, 10.0f, 0.001f);
	}

	{
		const float smallest_deg = DscCommon::MathAngle::RadianToDeg(DscCommon::MathAngle::SmallestDelta(
			DscCommon::MathAngle::DegToRadian(275.0f),
			DscCommon::MathAngle::DegToRadian(265.0f - 360.0f)
		));
		ok = TEST_UTIL_ALMOST_EQUAL_EPSILON(ok, smallest_deg, -10.0f, 0.001f);
	}

	return ok;
}

}//namespace

const bool DscCommonMathAngle()
{
	bool ok = true;

	ok &= TestRoundTripDegRad();
	ok &= TestSmallestAngle();

	return ok;
}