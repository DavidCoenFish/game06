#pragma once
#include <dsc_common/dsc_common.h>
#include "dsc_statistics.h"

/*
initial goal was to gather FPS, and try to do so in a manner suitable for onscreen debug info display
also want to be future compatable. 

event, like a "new frame" bookmark, or a "this view currently displaying 300 vertex", or "600 vertex in world"
some vauge performance data, like a scope object (start and end time) to track what was being done per frame...
derrived stats, like "FPS" (count of frames in the last second...)

class Bookmark : IEvent { timestamp } or is it a Bookmark<MethodReturningNewFrame>, or are Bookmarks constructed with a name?
class NewBookmarkPerSecond : IEvent { (observes Bookmark called NewFrame) }
class ValueInt : IEvent { int32 } // how may pos, how many links in simulation

use of unique_ptr feels like overkill, but wanted to get some hands on experence with unique_ptr
*/

namespace DscStatistics
{
	class IEvent; // GetName, GetDescription
	class IEventDerrived; // GetObservers (array of names that this events wants to observer), NotifyObservation, Update

	class EventStore
	{
	public:
		// what events we keep the data for
		void SetStoredEvents(const std::string& in_event_name);

		// is the name part of the IEvent?
		//void AddEvent(const std::string& in_name, std::unique_ptr<IEvent> in_event);
		void AddEvent(std::unique_ptr<IEvent> in_event);
		void AddEventDerrived(
			std::unique_ptr<IEventDerrived> in_event,
			const std::vector<std::string>& in_event_names_to_observe
			);

		void UpdateDerrived();

		//void VisitEventHistory(
		//	const std::function<void(const IEvent&)>& in_visitor
		//	);
		void VisitEvents(
			const std::function<void(const IEvent&)>& in_visitor
		);

	private:
		// todo: move to private implementation?
		std::set<std::string> _allowed_events;

		std::map<std::string, std::unique_ptr<IEvent>> _event_map;
		std::vector<IEventDerrived*> _derrived_events; // event map has ownership, just a shortcut to access Update method
		std::map<std::string, std::vector<IEventDerrived*>> _observer_map;
	};
}
