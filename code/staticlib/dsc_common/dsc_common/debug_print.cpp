#include "debug_print.h"

#if defined(_DEBUG)
const std::string DscCommon::DebugPrint::TabDepth(const int32_t in_depth)
{
	std::string result = {};
	for (int32 index = 0; index < in_depth; ++index)
	{
		result += "    ";
	}
	return result;
}
#endif //#if defined(_DEBUG)

