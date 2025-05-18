#include "common.h"
#include "log_system.h"
#include "log_consumer_output_debug_string.h"

namespace
{
DscCommon::LogSystem* sLogSystem = nullptr;

struct PendingMessage
{
	std::string mChannel;
	DscCommon::LogLevel mLevel;
	std::string mMessage;
};
std::vector<PendingMessage> sPendingMessageArray = {};

static const std::string FormatString(const char* const pFormat, va_list vaArgs)
{
	// reliably acquire the size
	// from a copy of the variable argument array
	// and a functionally reliable call to mock the formatting
	va_list vaArgsCopy;
	va_copy(vaArgsCopy, vaArgs);
	const int iLen = std::vsnprintf(NULL, 0, pFormat, vaArgsCopy);
	va_end(vaArgsCopy);

	// return a formatted string without risking memory mismanagement
	// and without assuming any compiler or platform specific behavior
	std::vector<char> zc(iLen + 1);
	std::vsnprintf(zc.data(), zc.size(), pFormat, vaArgs);

	std::string message(zc.data(), iLen); 

	return message;
}

} // namespace

std::unique_ptr<DscCommon::ILogConsumer>&& DscCommon::LogSystem::FactoryConsumerOutputDebugString()
{
	auto pConsumer = std::make_unique<LogConsumerOutputDebugString>();
	return std::move(pConsumer);
}

const std::string DscCommon::LogSystem::Printf(const char* const in_format, ... )
{
	va_list va_args;
	va_start(va_args, in_format);
	const std::string message = FormatString(in_format, va_args);
	va_end(va_args);
	return message;
}

void DscCommon::LogSystem::AddMessage(
	const std::string& in_channelName, 
	const LogLevel in_level,
	const char* const in_format, 
	... 
	)
{
	if (nullptr == sLogSystem)
	{
		va_list va_args;
		va_start(va_args, in_format);
		std::string message = FormatString(in_format, va_args);
		va_end(va_args);

		sPendingMessageArray.push_back(PendingMessage({
			in_channelName,
			in_level,
			message
			}));
	}
	else
	{
		if (sLogSystem->mGlobalLevel < in_level)
		{
			return;
		}
		const auto found = sLogSystem->mChannelLevels.find(in_channelName);
		if (found != sLogSystem->mChannelLevels.end())
		{
			if (found->second < in_level)
			{
				return;
			}
		}

		va_list va_args;
		va_start(va_args, in_format);
		std::string message = FormatString(in_format, va_args);
		va_end(va_args);

		sLogSystem->ProcessMessage(
			in_level, 
			message
			);
	}
	return;
}

DscCommon::LogSystem::LogSystem(std::vector<std::unique_ptr<ILogConsumer>>&& in_consumerArray)
: mConsumerArray(std::move(in_consumerArray))
{
	DSC_ASSERT(nullptr == sLogSystem, "singelton pattern");
	sLogSystem = this;

	ConsumePendingMessages();
	return;
}

DscCommon::LogSystem::LogSystem(std::unique_ptr<ILogConsumer>&& in_consumer)
{
	DSC_ASSERT(nullptr == sLogSystem, "singelton pattern");
	sLogSystem = this;

	AddConsumer(std::move(in_consumer));
	ConsumePendingMessages();
	return;
}

DscCommon::LogSystem::~LogSystem()
{
	sLogSystem = nullptr;
	return;
}

void DscCommon::LogSystem::SetLevel(const LogLevel in_level)
{
	mGlobalLevel = in_level;
	return;
}

const int32 DscCommon::LogSystem::AddConsumer(std::unique_ptr<ILogConsumer>&& in_consumer)
{
	const int32 result = static_cast<int32>(mConsumerArray.size());
	mConsumerArray.push_back(std::move(in_consumer));
	return result;
}

void DscCommon::LogSystem::SetChannelLevel(
	const std::string& in_channelName, 
	const LogLevel in_level
	)
{
	auto iter = mChannelLevels.find(in_channelName);
	if (iter != mChannelLevels.end())
	{
		//TODO: can you do this, or do you need mChannelLevels[in_channelName] = in_level;
		iter->second = in_level;
		// see todo above
		DSC_ASSERT(in_level == mChannelLevels[in_channelName], "are values in map iter ref or copy");
	}
	else
	{
		mChannelLevels.insert(std::pair<std::string, LogLevel>(in_channelName, in_level));
	}
	return;
}

void DscCommon::LogSystem::ConsumePendingMessages()
{
	for (const auto& iter: sPendingMessageArray)
	{
		if (mGlobalLevel < iter.mLevel)
		{
			continue;
		}
		const auto found = mChannelLevels.find(iter.mChannel);
		if (found != mChannelLevels.end())
		{
			if (found->second < iter.mLevel)
			{
				return;
			}
		}

		ProcessMessage(
			iter.mLevel,
			iter.mMessage
			);
	}
	sPendingMessageArray.clear();
	sPendingMessageArray.shrink_to_fit();
	return;
}

void DscCommon::LogSystem::ProcessMessage(
	const LogLevel in_level, 
	const std::string& in_message
	)
{
	for (auto& item : sLogSystem->mConsumerArray)
	{
		item->AddMessage(in_level, in_message);
	}
}

