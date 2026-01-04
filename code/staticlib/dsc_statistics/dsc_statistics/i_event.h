#pragma once
#include <dsc_common/dsc_common.h>
#include "dsc_statistics.h"

namespace DscStatistics
{
	class IEvent
	{
	public:
		virtual ~IEvent() {};

		virtual const std::string& GetName() const = 0;
		virtual const std::string GetDescription() const = 0;
	};
}
