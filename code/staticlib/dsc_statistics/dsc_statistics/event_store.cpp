#include "event_store.h"
#include "i_event.h"
#include "i_event_derrived.h"

void DscStatistics::EventStore::SetStoredEvents(const std::string& in_event_name)
{
	_allowed_events.insert(in_event_name);
	return;
}

void DscStatistics::EventStore::AddEvent(std::unique_ptr<IEvent> in_event)
{
	DSC_ASSERT(nullptr != in_event, "invalid input");
	const std::string& name = in_event->GetName();

	// notify observers
	const auto observer_iter = _observer_map.find(name);
	if (observer_iter != _observer_map.end())
	{
		for (const auto& iter : observer_iter->second)
		{
			iter->Notify(*in_event);
		}
	}

	// if event passes filter, add to event map
	if (_allowed_events.find(name) != _allowed_events.end())
	{
		// weak assumption that things comming into AddEvent are not overwritting Derrived events or other events that have raw ref...
		_event_map.emplace(name, std::move(in_event));
	}

	return;
}

void DscStatistics::EventStore::AddEventDerrived(
	std::unique_ptr<IEventDerrived> in_event,
	const std::vector<std::string>& in_event_names_to_observe
)
{
	DSC_ASSERT(nullptr != in_event, "invalid input");
	const std::string& name = in_event->GetName();

	DSC_ASSERT(_event_map.end() == _event_map.find(name), "if we are going to allow overwrite, may need to be careful about other palces that reference event raw pointer");
	IEventDerrived* const raw_event = in_event.get();
	_event_map.emplace(name, std::move(in_event));

	_derrived_events.push_back(raw_event);

	for (const auto& iter : in_event_names_to_observe)
	{
		auto found = _observer_map.find(iter);
		if (found != _observer_map.end())
		{
			found->second.push_back(raw_event);
		}
		else
		{
			std::vector<IEventDerrived*> observer_list = { raw_event };
			_observer_map.emplace(iter, std::move(observer_list));
		}
	}

	return;
}

void DscStatistics::EventStore::UpdateDerrived()
{
	for (const auto& iter : _derrived_events)
	{
		iter->Update();
	}

	return;
}

void DscStatistics::EventStore::VisitEvents(
	const std::function<void(const IEvent&)>& in_visitor
)
{
	for (const auto& iter : _event_map)
	{
		in_visitor(*iter.second);
	}

	return;
}

