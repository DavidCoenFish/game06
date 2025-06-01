#include <dsc_common/dsc_common.h>
#include "main.h"
#include "test_util.h"

#include "test\dsc_common_file_system.h"
#include "test\dsc_common_log_system.h"
#include "test\dsc_common_math_angle.h"
#include "test\dsc_common_utf8.h"
#include "test\std_map.h"

//int main(int argc, char* argv[], char* envp[])
int32 main(int32, char*, char*)
{
	TestUtil::AddTest(std::function<bool(void)>(DscCommonFileSystem));
	TestUtil::AddTest(std::function<bool(void)>(DscCommonLogSystem));
	TestUtil::AddTest(std::function<bool(void)>(DscCommonMathAngle));
	TestUtil::AddTest(std::function<bool(void)>(DscCommonUtf8));
	TestUtil::AddTest(std::function<bool(void)>(StdMap));

	int32 exitCode = 0;
	if (true != TestUtil::RunTests())
	{
		exitCode = -1;
	}

	std::cout << "<press enter to continue>" << std::endl;
	std::string output;
	std::getline(std::cin, output);
	return exitCode;
}

