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

		template <typename IN_TYPE>
		const std::string PrintType(const std::shared_ptr<IN_TYPE>&)
		{
			std::string result = "shared_ptr<";
			result += typeid(IN_TYPE).name();
			result += ">";
			return result;
		}

#endif // #if defined(_DEBUG)

	}// DebugPrint


#if defined(_DEBUG)
	template <typename IN_TYPE>
	struct DebugPrintNone {
		static const std::string Function(const IN_TYPE&)
		{
			return std::string();
		}
	};

	template <typename IN_TYPE>
	struct DebugPrintStandard {
		static const std::string Function(const IN_TYPE& in_value)
		{
			return std::to_string(in_value);
		}
	};
#endif // #if defined(_DEBUG)

}//DscCommon
