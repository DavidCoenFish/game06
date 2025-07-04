#pragma once
#include "dsc_common.h"

#include <dsc_common\dsc_common.h>
#include "i_log_consumer.h"

#if defined(DSC_LOG)
#define DSC_LOG_ERROR(TOPIC, FORMAT, ...) DscCommon::LogSystem::AddMessage(TOPIC, DscCommon::LogLevel::Error, FORMAT, ##__VA_ARGS__)
#define DSC_LOG_WARNING(TOPIC, FORMAT, ...) DscCommon::LogSystem::AddMessage(TOPIC, DscCommon::LogLevel::Warning, FORMAT, ##__VA_ARGS__)
#define DSC_LOG_INFO(TOPIC, FORMAT, ...) DscCommon::LogSystem::AddMessage(TOPIC, DscCommon::LogLevel::Info, FORMAT, ##__VA_ARGS__)
#define DSC_LOG_DIAGNOSTIC(TOPIC, FORMAT, ...) DscCommon::LogSystem::AddMessage(TOPIC, DscCommon::LogLevel::Diagnostic, FORMAT, ##__VA_ARGS__)

#else
#define DSC_LOG_ERROR(TOPIC, FORMAT, ...) (void)0
#define DSC_LOG_WARNING(TOPIC, FORMAT, ...) (void)0
#define DSC_LOG_INFO(TOPIC, FORMAT, ...) (void)0
#define DSC_LOG_DIAGNOSTIC(TOPIC, FORMAT, ...) (void)0
#endif

namespace DscCommon
{
class ILogConsumer;

class LogSystem
{
public:
	static std::unique_ptr<ILogConsumer> FactoryConsumerOutputDebugString();
	static const std::string Printf(const char* const in_format, ... );
	static void AddMessage(
		const std::string& in_channelName, 
		const LogLevel in_level,
		const char* const in_format, 
		... 
		);

	// so, when the app shutsdown, check that all log messages where consumed?
	//static void OnAppShutdownCheckAllMessagesConsumed();

	//todo, a ctor that takes a std::vector<std::unique_ptr<ILogConsumer>>?

	LogSystem(const LogLevel in_globalLevel, std::vector<std::unique_ptr<ILogConsumer>>&& in_consumerArray);
	LogSystem(const LogLevel in_globalLevel, std::unique_ptr<ILogConsumer>&& in_consumer = FactoryConsumerOutputDebugString());
	~LogSystem();
	const int32 AddConsumer(std::unique_ptr<ILogConsumer>&& in_consumer);
	void SetLevel(const LogLevel in_level);
	void SetChannelLevel(
		const std::string& in_channelName, 
		const LogLevel in_level
		);

private:
	void ConsumePendingMessages();
	void ProcessMessage(
		const LogLevel in_level, 
		const std::string& in_message
		);

private:
	// filter ALL messages with this
	LogLevel mGlobalLevel = LogLevel::None;
	// filter messages for channel against this
	std::map<std::string, LogLevel> mChannelLevels;

	std::vector<std::unique_ptr<ILogConsumer>> mConsumerArray;

}; //LogSystem
} //DscCommon