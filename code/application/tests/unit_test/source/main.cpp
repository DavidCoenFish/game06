#include "unit_test.h"
#include <dsc_common/dsc_common.h>
#include "main.h"
#include "test_util.h"

#include "test\dsc_common_file_system.h"
#include "test\dsc_common_log_system.h"
#include "test\dsc_common_math_angle.h"
#include "test\dsc_common_utf8.h"
#include "test\dsc_dag_2.h"
#include "test\dsc_dag_container.h"
#include "test\dsc_dag_group.h"
#include "test\dsc_data.h"
#include "test\std_any.h"
#include "test\std_map.h"
#include "test\std_set.h"
#include "test\std_type_info.h"
#include "test\std_unique.h"

namespace
{
	struct CmdLineParam
	{
		bool _unattended = false;
	};

	void ParseCmdLine(CmdLineParam& out_cmd_line_param, const int32 in_argc, char** in_argv)
	{
		for (int32 index = 0; index < in_argc; ++index)
		{
			if (0 == strcmp(in_argv[index], "-unattended"))
			{
				out_cmd_line_param._unattended = true;
			}
		}
	}
};


//int main(int argc, char* argv[], char* envp[])
int32 main(int32 in_argc, char** in_argv, char*)
{
	std::cout << "unit_test.exe" << std::endl;

	CmdLineParam cmdLineParam;
	ParseCmdLine(cmdLineParam, in_argc, in_argv);

	TestUtil::AddTest(std::function<bool(void)>(DscCommonFileSystem));
	TestUtil::AddTest(std::function<bool(void)>(DscCommonLogSystem));
	TestUtil::AddTest(std::function<bool(void)>(DscCommonMathAngle));
	TestUtil::AddTest(std::function<bool(void)>(DscCommonUtf8));
	TestUtil::AddTest(std::function<bool(void)>(DscDag2Test));
	TestUtil::AddTest(std::function<bool(void)>(DscDagContainer));
	TestUtil::AddTest(std::function<bool(void)>(DscDagGroup));
	TestUtil::AddTest(std::function<bool(void)>(TestDscData));
	TestUtil::AddTest(std::function<bool(void)>(StdAny));
	TestUtil::AddTest(std::function<bool(void)>(StdMap));
	TestUtil::AddTest(std::function<bool(void)>(StdSet));
	TestUtil::AddTest(std::function<bool(void)>(StdTypeInfo));
	TestUtil::AddTest(std::function<bool(void)>(StdUnique));

	int32 exitCode = 0;
	if (true != TestUtil::RunTests())
	{
		exitCode = -1;
	}

	if (false == cmdLineParam._unattended)
	{
		std::cout << "<press enter to continue>" << std::endl;
		std::string output;
		std::getline(std::cin, output);
	}

	return exitCode;
}

