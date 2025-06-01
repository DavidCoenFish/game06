#pragma once

#include <dsc_common\dsc_common.h>
#include "i_log_consumer.h"

namespace DscCommon
{
class IFileSystemOverlay;

class LogConsumerOutputDebugString : public ILogConsumer
{
public:
	virtual void AddMessage(const LogLevel in_level, const std::string& in_message) override;

}; //LogConsumerOutputDebugString
} //DscCommon