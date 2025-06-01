#include <dsc_common\dsc_common.h>
#include "i_log_consumer.h"

namespace
{
	const std::string sLevelText[DscCommon::LogLevel::Count] = 
	{
		"[None]:",
		"[ERROR]:",
		"[Warning]:",
		"[Info]:",
		"[Diaganostic]:",
	};
};

const std::string& DscCommon::ILogConsumer::GetLogTextForLevel(const LogLevel in_level)
{
	return sLevelText[in_level];
}

