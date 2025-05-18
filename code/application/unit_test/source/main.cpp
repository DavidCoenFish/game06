#include <dsc_common\common.h>
#include "main.h"

#include "dsc_common_math_angle.h"
#include "unit_test_util.h"

//int main(int argc, char* argv[], char* envp[])
int32 main(int32, char*, char*)
{
	UnitTestUtil::AddTest(std::function<bool (void)>(DscCommonMathAngle));

	int32 exitCode = 0;
	if (true != UnitTestUtil::RunTests())
	{
		exitCode = -1;
	}

	return exitCode;
}

