#include <dsc_common/dsc_common.h>
#include "std_any.h"
#include "test_util.h"

namespace
{
const bool TestBasic()
{
	std::any v0 = 123;

	bool ok = true;
	//ok = TEST_UTIL_EQUAL(ok, v0.type(), typeid(int));
	ok = TEST_UTIL_EQUAL(ok, 123, std::any_cast<int>(v0));
	ok = TEST_UTIL_EQUAL(ok, true, v0.has_value());

	v0.reset();
	ok = TEST_UTIL_EQUAL(ok, false, v0.has_value());

	return ok;
}

const bool TestString()
{
	std::any v0 = std::string("test");

	bool ok = true;
	//ok = TEST_UTIL_EQUAL(ok, v0.type(), typeid(int));
	ok = TEST_UTIL_EQUAL_STRING(ok, std::string("test"), std::any_cast<std::string>(v0));
	ok = TEST_UTIL_EQUAL(ok, true, v0.has_value());

	v0.reset();
	ok = TEST_UTIL_EQUAL(ok, false, v0.has_value());

	return ok;
}

const bool TestPair()
{
	typedef std::pair<std::string, std::string> TPair;
	std::any v0 = TPair(std::string("key"), std::string("value"));

	bool ok = true;
	//ok = TEST_UTIL_EQUAL(ok, v0.type(), typeid(int));
	ok = TEST_UTIL_EQUAL_STRING(ok, std::string("key"), std::any_cast<TPair>(v0).first);
	ok = TEST_UTIL_EQUAL_STRING(ok, std::string("value"), std::any_cast<TPair>(v0).second);

	ok = TEST_UTIL_EQUAL(ok, true, v0.has_value());

	v0.reset();
	ok = TEST_UTIL_EQUAL(ok, false, v0.has_value());

	return ok;

}

}//namespace

const bool StdAny()
{
	bool ok = true;
	ok &= TestBasic();
	ok &= TestString();
	ok &= TestPair();

	return ok;
}