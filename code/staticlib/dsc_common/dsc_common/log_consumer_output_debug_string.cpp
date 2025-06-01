#include <dsc_common\dsc_common.h>
#include "log_consumer_output_debug_string.h"

namespace
{
} // namespace

void DscCommon::LogConsumerOutputDebugString::AddMessage(const LogLevel in_level, const std::string& in_message)
{
	const std::string fullMessage = GetLogTextForLevel(in_level) + in_message;
	OutputDebugString(
		std::wstring(fullMessage.begin(), fullMessage.end()).c_str()
		);
}
