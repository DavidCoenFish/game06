#pragma once

#include <dsc_common\common.h>

namespace TestUtil
{
	template<typename TYPE>
	const bool Equal(const bool ok, const TYPE valueLhs, const TYPE valueRhs, const std::string& fileName, const int lineNumber)
	{
		if (valueLhs != valueRhs)
		{
			std::cerr << "Equal failed, lhs:" << valueLhs << " rhs:" << valueRhs << " file:" << fileName << " line:" << lineNumber << std::endl;
			return false;
		}
		return ok;
	}

	template<typename TYPE>
	const bool EqualVector(const bool ok, const std::vector<TYPE>& valueLhs, const std::vector<TYPE>& valueRhs, const std::string& fileName, const int lineNumber)
	{
		bool match = false;
		if (valueLhs.size() == valueRhs.size())
		{
			match = true;
			for (size_t index = 0; index < valueRhs.size(); ++index)
			{
				if (valueLhs[index] != valueRhs[index])
				{
					match = false;
					std::cerr << "Equal Vector failed, lhs:" << valueLhs[index] << " rhs:" << valueRhs[index] << " index:" << index << " file:" << fileName << " line:" << lineNumber << std::endl;
					break;
				}
			}
		}
		else
		{
			std::cerr << "Equal Vector failed, lhs size:" << valueLhs.size() << " rhs size:" << valueRhs.size() << " file:" << fileName << " line:" << lineNumber << std::endl;
		}

		if (false == match)
		{
			return false;
		}
		return ok;
	}

	template<typename TYPE>
	const bool NotEqual(const bool ok, const TYPE valueLhs, const TYPE valueRhs, const std::string& fileName, const int lineNumber)
	{
		if (valueLhs == valueRhs)
		{
			std::cerr << "Not equal failed, lhs:" << valueLhs << " rhs:" << valueRhs << " file:" << fileName << " line:" << lineNumber << std::endl;
			return false;
		}
		return ok;
	}

	const bool AlmostEqual(const bool ok, const float valueLhs, const float valueRhs, const std::string& fileName, const int lineNumber); 
	//expect not almost equal
	//expect null
	//expect not null

	void AddTest(const std::function<bool()>& in_test);

	const bool RunTests();
}

#define TEST_UTIL_EQUAL(OK, LHS, RHS) TestUtil::Equal(OK, LHS, RHS, __FILE__, __LINE__);
#define TEST_UTIL_NOT_EQUAL(OK, LHS, RHS) TestUtil::NotEqual(OK, LHS, RHS, __FILE__, __LINE__);
#define TEST_UTIL_ALMOST_EQUAL(OK, LHS, RHS) TestUtil::AlmostEqual(OK, LHS, RHS, __FILE__, __LINE__);
#define TEST_UTIL_EQUAL_VECTOR(OK, LHS, RHS) TestUtil::EqualVector(OK, LHS, RHS, __FILE__, __LINE__);

