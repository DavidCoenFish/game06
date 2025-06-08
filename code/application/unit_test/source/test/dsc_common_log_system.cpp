#include "..\unit_test.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/log_system.h>
#include <dsc_common/i_log_consumer.h>
#include "dsc_common_log_system.h"
#include "test_util.h"

namespace
{
class LogConsumerMemory : public DscCommon::ILogConsumer
{
public:
	struct Data
	{
		DscCommon::LogLevel mLevel;
		std::string mMessages;
	};

	LogConsumerMemory(std::shared_ptr<std::vector<Data>>& in_data) : mDataArray(in_data){}
	virtual void AddMessage(const DscCommon::LogLevel in_level, const std::string& in_message) override
	{
		mDataArray->push_back(Data({in_level, in_message}));
	}

public:
	std::shared_ptr<std::vector<Data>> mDataArray;
};

const bool TestDrain()
{
	DscCommon::LogSystem::AddMessage("test", DscCommon::Info, "hello %s", "human");

	//std::vector<LogConsumerMemory::Data> dataArray;
	std::shared_ptr<std::vector<LogConsumerMemory::Data>> dataArray = std::make_shared<std::vector<LogConsumerMemory::Data>>();
	{
		DscCommon::LogSystem logSystem(DscCommon::Diagnostic, std::make_unique<LogConsumerMemory>(dataArray));
	}

	bool ok = true;
	ok = TEST_UTIL_EQUAL(ok, (int32)1, (int32)dataArray->size());

	return ok;
}

const bool TestDrainMany()
{
	DscCommon::LogSystem::AddMessage("test", DscCommon::Error, "hello %s", "error");
	DscCommon::LogSystem::AddMessage("test", DscCommon::Warning, "hello %s", "warning");
	DscCommon::LogSystem::AddMessage("test", DscCommon::Info, "hello %s", "info");
	DscCommon::LogSystem::AddMessage("test", DscCommon::Diagnostic, "hello %s", "diagnostic");

	bool ok = true;
	std::shared_ptr<std::vector<LogConsumerMemory::Data>> dataArray = std::make_shared<std::vector<LogConsumerMemory::Data>>();
	{
		DscCommon::LogSystem logSystem(DscCommon::Info, std::make_unique<LogConsumerMemory>(dataArray));

		ok = TEST_UTIL_EQUAL(ok, (int32)3, (int32)dataArray->size());

		DscCommon::LogSystem::AddMessage("test", DscCommon::Diagnostic, "hello %s", "diagnostic2");
		DscCommon::LogSystem::AddMessage("test", DscCommon::Info, "hello %s", "info2");
		DscCommon::LogSystem::AddMessage("test", DscCommon::Info, "hello %s", "info3");

		ok = TEST_UTIL_EQUAL(ok, (int32)5, (int32)dataArray->size());
	}

	return ok;
}

}//namespace

const bool DscCommonLogSystem()
{
	bool ok = true;
	ok &= TestDrain();
	ok &= TestDrainMany();

	return ok;
}