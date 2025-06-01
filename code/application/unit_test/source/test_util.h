#pragma once

#include <dsc_common/dsc_common.h>

namespace TestUtil
{
	template<typename TYPE>
	const bool Equal(const bool in_ok, const TYPE in_valueLhs, const TYPE in_valueRhs, const std::string& in_fileName, const int in_lineNumber)
	{
		if (in_valueLhs != in_valueRhs)
		{
			std::cerr << "Equal failed, lhs:" << in_valueLhs << " rhs:" << in_valueRhs << " file:" << in_fileName << " line:" << in_lineNumber << std::endl;
			return false;
		}
		return in_ok;
	}

	template<typename TYPE>
	const bool EqualVector(const bool in_ok, const std::vector<TYPE>& in_valueLhs, const std::vector<TYPE>& in_valueRhs, const std::string& in_fileName, const int in_lineNumber)
	{
		bool match = false;
		if (in_valueLhs.size() == in_valueRhs.size())
		{
			match = true;
			for (size_t index = 0; index < in_valueRhs.size(); ++index)
			{
				if (in_valueLhs[index] != in_valueRhs[index])
				{
					match = false;
					std::cerr << "Equal Vector failed, lhs:" << in_valueLhs[index] << " rhs:" << in_valueRhs[index] << " index:" << index << " file:" << in_fileName << " line:" << in_lineNumber << std::endl;
					break;
				}
			}
		}
		else
		{
			std::cerr << "Equal Vector failed, lhs size:" << in_valueLhs.size() << " rhs size:" << in_valueRhs.size() << " file:" << in_fileName << " line:" << in_lineNumber << std::endl;
		}

		if (false == match)
		{
			return false;
		}
		return in_ok;
	}

	template<typename TYPE>
	const bool NotEqual(const bool in_ok, const TYPE in_valueLhs, const TYPE in_valueRhs, const std::string& in_fileName, const int in_lineNumber)
	{
		if (in_valueLhs == in_valueRhs)
		{
			std::cerr << "Not equal failed, lhs:" << in_valueLhs << " rhs:" << in_valueRhs << " file:" << in_fileName << " line:" << in_lineNumber << std::endl;
			return false;
		}
		return in_ok;
	}

	const bool AlmostEqual(const bool in_ok, const float in_valueLhs, const float in_valueRhs, const std::string& in_fileName, const int in_lineNumber);
	const bool EqualString(const bool in_ok, const std::string& in_valueLhs, const std::string& in_valueRhs, const std::string& in_fileName, const int in_lineNumber);

	void AddTest(const std::function<bool()>& in_test);

	const bool RunTests();
}

#define TEST_UTIL_EQUAL(OK, LHS, RHS) TestUtil::Equal(OK, LHS, RHS, __FILE__, __LINE__);
#define TEST_UTIL_NOT_EQUAL(OK, LHS, RHS) TestUtil::NotEqual(OK, LHS, RHS, __FILE__, __LINE__);
#define TEST_UTIL_ALMOST_EQUAL(OK, LHS, RHS) TestUtil::AlmostEqual(OK, LHS, RHS, __FILE__, __LINE__);
#define TEST_UTIL_EQUAL_VECTOR(OK, LHS, RHS) TestUtil::EqualVector(OK, LHS, RHS, __FILE__, __LINE__);
#define TEST_UTIL_EQUAL_STRING(OK, LHS, RHS) TestUtil::EqualString(OK, LHS, RHS, __FILE__, __LINE__);

