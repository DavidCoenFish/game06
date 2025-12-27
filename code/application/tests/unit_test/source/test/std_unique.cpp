#include "std_map.h"
#include <dsc_common/dsc_common.h>
#include "test_util.h"

namespace
{
	// from google ai answer
	class Base {
	public:
		virtual ~Base() = default;
		virtual void someMethod() { /* ... */ }
	};

	class Derived : public Base {
	public:
		void someMethod() override { /* ... */ }
	};

const bool TestDowncastAssign()
{
	std::unique_ptr<Base> basePtr = std::make_unique<Derived>();

	bool ok = true;
	ok = TEST_UTIL_EQUAL(ok, true, nullptr != basePtr.get());

	return ok;
}

}//namespace

const bool StdUnique()
{
	bool ok = true;
	ok &= TestDowncastAssign();

	return ok;
}