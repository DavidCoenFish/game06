#pragma once
#include <dsc_common/dsc_common.h>
#include "dsc_statistics.h"
#include "i_event.h"


namespace DscStatistics
{
	class IEventDerrived : public IEvent
	{
	public:
		//virtual void CollectObservers(std::vector<std::string>& out_event_name_array) const = 0;
		virtual void Notify(const IEvent& in_event) = 0;
		virtual void Update() = 0;

	};
}
