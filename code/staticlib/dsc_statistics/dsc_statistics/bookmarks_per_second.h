#pragma once
#include <dsc_common/dsc_common.h>
#include "dsc_statistics.h"
#include "i_event_derrived.h"

/*
wanted a way to gather a FPS value
*/
namespace DscStatistics
{
	class BookmarksPerSecond : public IEventDerrived
	{
	public:
		BookmarksPerSecond() = delete;
		BookmarksPerSecond& operator=(const BookmarksPerSecond&) = delete;
		BookmarksPerSecond(const BookmarksPerSecond&) = delete;

		static std::unique_ptr<BookmarksPerSecond> Factory(
			const std::string& in_name//,
			//const std::string& in_bookmark_name_to_observe
		);

		BookmarksPerSecond(
			const std::string& in_name//,
			//const std::string& in_bookmark_name_to_observe
		);

	private:

		virtual const std::string& GetName() const override { return _name; }
		virtual const std::string GetDescription() const override;
		//virtual void CollectObservers(std::vector<std::string>& out_event_name_array) const override;
		virtual void Notify(const IEvent& in_event) override;
		virtual void Update() override;

	private:
		const std::string _name;
		//const std::string _bookmark_name_to_observe;
		//steady_clock
		std::chrono::system_clock::time_point _time_point_second_start;
		bool _bookmarks_per_last_second_valid = false;
		int32 _bookmarks_per_last_second = 0;
		int32 _bookmarks_so_far_this_second = 0;

	};
}
