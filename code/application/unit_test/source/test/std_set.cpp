#include "std_set.h"
#include <dsc_common/dsc_common.h>
#include "test_util.h"

namespace
{
	/*
	//this doesn't work, std::set is not fifo/ order preserving
const bool TestOrder()
{
	std::set<std::string> set = {};

	set.insert("one");
	set.insert("two");
	set.insert("three");
	set.insert("four");
	set.insert("five");
	set.insert("six");

	bool ok = true;
	auto iter = set.begin();
	ok = TEST_UTIL_EQUAL(ok, std::string("one"), *iter);
	++iter;
	ok = TEST_UTIL_EQUAL(ok, std::string("two"), *iter);
	++iter;
	ok = TEST_UTIL_EQUAL(ok, std::string("three"), *iter);
	++iter;
	ok = TEST_UTIL_EQUAL(ok, std::string("four"), *iter);
	++iter;
	ok = TEST_UTIL_EQUAL(ok, std::string("five"), *iter);
	++iter;
	ok = TEST_UTIL_EQUAL(ok, std::string("six"), *iter);

	return true;
}
	*/

}//namespace

const bool StdSet()
{
	bool ok = true;

	//this doesn't work, std::set is not fifo/ order preserving
	//ok &= TestOrder();

	return ok;
}