#include <dsc_common\common.h>
#include <dsc_common\log_system.h>
#include <dsc_common\i_log_consumer.h>
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

}//namespace

const bool DscCommonLogSystem()
{
	bool ok = true;
	ok &= TestDrain();

	return ok;
}