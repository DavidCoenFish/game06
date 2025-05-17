#include <dsc_common\common.h>
#include "unit_test_util.h"

namespace
{
	std::vector<std::function<bool()>>& GetTestArray()
	{
		static std::vector<std::function<bool()>> sTestArray;
		return sTestArray;
	}
};

const bool UnitTestUtil::AlmostEqual(const bool ok, const float valueLhs, const float valueRhs, const std::string& fileName, const int lineNumber)
{
	if (std::numeric_limits<float>::epsilon() < std::abs(valueLhs - valueRhs))
	{
		std::cerr << "AlmostEqual failed, lhs:" << valueLhs << " rhs:" << valueRhs << " file:" << fileName << " line:" << lineNumber << std::endl;
		return ok;
	}
	return false;
}

void UnitTestUtil::AddTest(const std::function<bool()>& in_test)
{
	GetTestArray().push_back(in_test);
}

const bool UnitTestUtil::RunTests()
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
