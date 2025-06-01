#include <dsc_common\dsc_common.h>
#include "log_consumer_std_out.h"

void DscCommon::LogConsumerStdOut::AddMessage(const DscCommon::LogLevel in_level, const std::string& in_message)
{
	if (in_level == LogLevel::Error)
	{
		std::cerr << GetLogTextForLevel(in_level) << in_message << std::endl;
	}
	else
	{
		std::cout << GetLogTextForLevel(in_level) << in_message << std::endl;
	}
}
