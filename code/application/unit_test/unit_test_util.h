#pragma once

namespace UnitTestUtil
{
	//expect equals
	//expect not equal
	//expect almost equals
	//expect not almost equal
	//expect null
	//expect null null

	//add test
	void AddTest(std::function<bool()>& in_test);
	//visit tests
	void RunTests();
}
