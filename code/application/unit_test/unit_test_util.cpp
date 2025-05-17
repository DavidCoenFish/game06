#include <dsc_common\common.h>
#include "unit_test_util.h"

namespace
{
std::vector<std::function<bool()>> sTestArray;
};

void UnitTestUtil::AddTest(std::function<bool()>& in_test)
{
	sTestArray.push_back(in_test);
}

void UnitTestUtil::RunTests()
{
	int32 count_total = 0;
	int32 count_pass = 0;
	for(const auto& item : sTestArray)
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

}
