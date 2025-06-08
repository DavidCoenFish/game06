#include "..\unit_test.h"
#include <dsc_common/dsc_common.h>
#include "std_map.h"
#include "test_util.h"

namespace
{
const bool TestFoundValueChange()
{
	std::map<int32, int32> map = {};

	map.insert(std::pair<int32, int32>(0,0));
	auto iter = map.find(0);
	iter->second = 1;

	bool ok = true;
	ok = TEST_UTIL_EQUAL(ok, 1, map[0]);

	return true;
}

}//namespace

const bool StdMap()
{
	bool ok = true;
	ok &= TestFoundValueChange();

	return ok;
}