#pragma once

#include <dsc_common\dsc_common.h>

namespace DscCommon
{
enum LogLevel : uint32
{
	None, // used to filter out all messages, messages sent at None level should assert
	Error,
	Warning,
	Info,
	Diagnostic,

	Count
};

class ILogConsumer
{
public:
	static const std::string& GetLogTextForLevel(const LogLevel in_level);

	ILogConsumer(){};
	virtual ~ILogConsumer(){};

	virtual void AddMessage(const LogLevel in_level, const std::string& in_message) = 0;

}; //ILogConsumer
} //DscCommon