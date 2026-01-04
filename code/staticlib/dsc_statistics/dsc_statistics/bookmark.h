#pragma once
#include <dsc_common/dsc_common.h>
#include "dsc_statistics.h"
#include "i_event.h"

/*
a bookmark in time of when something happened, alternative name could have been "Event"?
*/
namespace DscStatistics
{
	class Bookmark : public IEvent
	{
	public:
		Bookmark() = delete;
		Bookmark& operator=(const Bookmark&) = delete;
		Bookmark(const Bookmark&) = delete;

		static std::unique_ptr<Bookmark> Factory(
			const std::string& in_name,
			const std::chrono::system_clock::time_point& in_time_point
		);

		Bookmark(
			const std::string& in_name,
			const std::chrono::system_clock::time_point& in_time_point
		);

		const std::chrono::system_clock::time_point& GetTimePoint() const { return _time_point; }

	private:

		virtual const std::string& GetName() const override { return _name; }
		virtual const std::string GetDescription() const override;

	private:
		const std::string _name;
		//const std::chrono::steady_clock::time_point _time_point;
		const std::chrono::system_clock::time_point _time_point;

		
	};
}
