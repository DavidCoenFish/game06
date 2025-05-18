#pragma once

#include <dsc_common\common.h>

namespace TestUtil
{
	//expect equals
	//expect not equal
	//expect almost equals
	template<typename TYPE>
	const bool Equal(const bool ok, const TYPE valueLhs, const TYPE valueRhs, const std::string& fileName, const int lineNumber)
	{
		if (valueLhs != valueRhs)
		{
			std::cerr << "Equal failed, lhs:" << valueLhs << " rhs:" << valueRhs << " file:" << fileName << " line:" << lineNumber << std::endl;
			return ok;
		}
		return false;
	}
	const bool AlmostEqual(const bool ok, const float valueLhs, const float valueRhs, const std::string& fileName, const int lineNumber); 
	//expect not almost equal
	//expect null
	//expect null null

	void AddTest(const std::function<bool()>& in_test);

	const bool RunTests();
}

#define TEST_UTIL_EQUAL(OK, LHS, RHS) TestUtil::Equal(OK, LHS, RHS, __FILE__, __LINE__);
#define TEST_UTIL_ALMOST_EQUAL(OK, LHS, RHS) TestUtil::AlmostEqual(OK, LHS, RHS, __FILE__, __LINE__);

