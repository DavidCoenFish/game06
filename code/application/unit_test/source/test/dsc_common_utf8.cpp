#include <dsc_common/dsc_common.h>
#include <dsc_common/utf8.h>
#include "dsc_common_utf8.h"
#include "test_util.h"

namespace
{
const bool TestSanity()
{
	const std::string step0 = { "\x0a\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xb8\x96\xe7\x95\x8c" };
	const std::wstring step1 = Utf8::Utf8ToUtf16(step0);
	const std::string step2 = Utf8::Utf16ToUtf8(step1);

	bool ok = true;
	ok = TEST_UTIL_EQUAL_STRING(ok, step0, step2);

	return true;
}

}//namespace

const bool DscCommonUtf8()
{
	bool ok = true;

	ok &= TestSanity();

	return ok;
}