#include "std_type_info.h"
#include <dsc_common/dsc_common.h>
#include "test_util.h"

namespace
{
const bool TestSanity()
{
	bool ok = true;
	ok = TEST_UTIL_EQUAL(ok, true, typeid(int) == typeid(int));

	return true;
}

}//namespace

const bool StdTypeInfo()
{
	bool ok = true;
	ok &= TestSanity();

	return ok;
}