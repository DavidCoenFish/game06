#include "std_type_info.h"
#include <dsc_common/dsc_common.h>
#include "test_util.h"

namespace
{
const bool TestSanity()
{
	bool ok = true;
	ok = TEST_UTIL_EQUAL(ok, true, typeid(int) == typeid(int));

	return ok;
}

enum class TEnumA
{
	TOne,
	TTwo,
	TCount
};

enum class TEnumB
{
	TThree,
	TFour,
	TCount
};

// confirm that enum are distinct types
const bool TestEnum()
{
	bool ok = true;
	ok = TEST_UTIL_EQUAL(ok, true, typeid(TEnumA) == typeid(TEnumA));
	ok = TEST_UTIL_EQUAL(ok, false, typeid(TEnumA) == typeid(TEnumB));
	ok = TEST_UTIL_EQUAL(ok, false, typeid(TEnumB) == typeid(TEnumA));
	ok = TEST_UTIL_EQUAL(ok, true, typeid(TEnumB) == typeid(TEnumB));

	return ok;
}

}//namespace

const bool StdTypeInfo()
{
	bool ok = true;
	ok &= TestSanity();
	ok &= TestEnum();

	return ok;
}