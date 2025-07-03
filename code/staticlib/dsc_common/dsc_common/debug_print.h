#pragma once
#include "dsc_common.h"

namespace DscCommon
{
	namespace DebugPrint
	{
#if defined(_DEBUG)
		const std::string TabDepth(const int32_t in_depth);

		template <typename IN_TYPE>
		const std::string PrintType(const IN_TYPE& in_value)
		{
			return std::to_string(in_value);
		}
#endif // #if defined(_DEBUG)

	}// DebugPrint
}//DscCommon
