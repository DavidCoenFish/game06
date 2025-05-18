#include "common.h"
#include "log_consumer_output_debug_string.h"

namespace
{

//TODO move to it's own file if needed anywhere else
const std::wstring stringToWstring(const std::string& str) {
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

} // namespace

void DscCommon::LogConsumerOutputDebugString::AddMessage(const LogLevel in_level, const std::string& in_message)
{
	OutputDebugString(
		stringToWstring(GetLogTextForLevel(in_level) + in_message).c_str()
		);
}
