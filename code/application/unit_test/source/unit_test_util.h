#pragma once

#include <dsc_common\common.h>

namespace UnitTestUtil
{
	//expect equals
	//expect not equal
	//expect almost equals
	const bool AlmostEqual(const bool ok, const float valueLhs, const float valueRhs, const std::string& fileName, const int lineNumber); 
	//expect not almost equal
	//expect null
	//expect null null

	void AddTest(const std::function<bool()>& in_test);

	const bool RunTests();
}

#define UNIT_TEST_UTIL_ALMOST_EQUAL(OK, LHS, RHS) UnitTestUtil::AlmostEqual(OK, LHS, RHS, __FILE__, __LINE__);

