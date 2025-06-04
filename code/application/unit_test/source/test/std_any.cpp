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

const bool TestComparison()
{
	std::any v0 = 3;
	bool ok = true;
	//ok = TEST_UTIL_EQUAL(ok, v0.type(), typeid(int));
	std::any v1 = 1.4f;
	//ok = TEST_UTIL_EQUAL(ok, v0.type(), typeid(float));
	//std::cout << v0.type() << std::endl;
	//const std::type_info& r0 = v0.type();
	ok = TEST_UTIL_EQUAL(ok, false, v0.type() == v1.type());
	ok = TEST_UTIL_EQUAL(ok, true, v0.type() == typeid(int));
	ok = TEST_UTIL_EQUAL(ok, true, v1.type() == typeid(float));
	std::any v2 = 3;
	//ok = TEST_UTIL_EQUAL(ok, false, v0 == v1);
	//ok = TEST_UTIL_EQUAL(ok, true, v0 == v2);
	std::any v3 = {};
	ok = TEST_UTIL_EQUAL(ok, false, v0.type() == v3.type());
	//std::count << std::to_string(r0) << std::endl;

	ok = TEST_UTIL_EQUAL(ok, true, std::any_cast<int>(v0) == std::any_cast<int>(v2));
	ok = TEST_UTIL_EQUAL(ok, 3, std::any_cast<int>(v0));
	ok = TEST_UTIL_EQUAL(ok, 3, std::any_cast<int>(v2));

	return ok;
}

}//namespace

const bool StdAny()
{
	bool ok = true;
	ok &= TestBasic();
	ok &= TestString();
	ok &= TestPair();
	ok &= TestComparison();

	return ok;
}