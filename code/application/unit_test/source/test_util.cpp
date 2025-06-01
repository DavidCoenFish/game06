#include <dsc_common/dsc_common.h>
#include "test_util.h"

namespace
{
	std::vector<std::function<bool()>>& GetTestArray()
	{
		static std::vector<std::function<bool()>> sTestArray;
		return sTestArray;
	}
};

const bool TestUtil::AlmostEqual(const bool in_ok, const float in_valueLhs, const float in_valueRhs, const std::string& in_fileName, const int in_lineNumber)
{
	if (std::numeric_limits<float>::epsilon() < std::abs(in_valueLhs - in_valueRhs))
	{
		std::cerr << "AlmostEqual failed, lhs:" << in_valueLhs << " rhs:" << in_valueRhs << " file:" << in_fileName << " line:" << in_lineNumber << std::endl;
		return false;
	}
	return in_ok;
}

const bool TestUtil::EqualString(const bool in_ok, const std::string& in_valueLhs, const std::string& in_valueRhs, const std::string& in_fileName, const int in_lineNumber)
{
	if (in_valueLhs != in_valueRhs)
	{
		std::cerr << "EqualString failed, lhs:" << in_valueLhs << " rhs:" << in_valueRhs << " file:" << in_fileName << " line:" << in_lineNumber << std::endl;
		return false;
	}
	return in_ok;
}

void TestUtil::AddTest(const std::function<bool()>& in_test)
{
	GetTestArray().push_back(in_test);
}

const bool TestUtil::RunTests()
{
	int32 count_total = 0;
	int32 count_pass = 0;
	for(const auto& item : GetTestArray())
	{
		try
		{
			if (true == item())
			{
				count_pass += 1;
			}
		}
		catch(...)
		{
			std::cerr << "exception thrown" << std::endl;
		}
		count_total += 1;
	}

	std::cout << "Pass:" << count_pass << " of tests:" <<  count_total << std::endl;

	return (count_total == count_pass);
}
