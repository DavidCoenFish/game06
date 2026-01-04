#include "bookmarks_per_second.h"

std::unique_ptr<DscStatistics::BookmarksPerSecond> DscStatistics::BookmarksPerSecond::Factory(
	const std::string& in_name//,
	//const std::string& in_bookmark_name_to_observe
)
{
	return std::make_unique<BookmarksPerSecond>(in_name);
}

DscStatistics::BookmarksPerSecond::BookmarksPerSecond(
	const std::string& in_name//,
	//const std::string& in_bookmark_name_to_observe
) : _name(in_name) 
{
	// nop
}

const std::string DscStatistics::BookmarksPerSecond::GetDescription() const
{
	return _name + std::string(": ") + std::to_string(_bookmarks_per_last_second_valid ? _bookmarks_per_last_second : _bookmarks_so_far_this_second);
}

void DscStatistics::BookmarksPerSecond::Notify(const IEvent& in_event)
{
	DSC_UNUSED(in_event);
	Update();
	_bookmarks_so_far_this_second += 1;
}

void DscStatistics::BookmarksPerSecond::Update()
{
	//const auto now = std::chrono::high_resolution_clock::now();
	const auto now = std::chrono::system_clock::now();
	
	const float delta = std::chrono::duration<float, std::ratio<1, 1>>(now - _time_point_second_start).count();

	if (delta < 1.0f)
	{
		// nop
		return;
	}
	else if (2.0f < delta) // something happened, so hitch or suspend? clear values
	{
		_bookmarks_per_last_second_valid = false;
		_bookmarks_per_last_second = 0;
		_bookmarks_so_far_this_second = 0;
		_time_point_second_start = now;
	}
	else // delta since last time bookmark is over a second, and less than two
	{
		_bookmarks_per_last_second_valid = true;
		_bookmarks_per_last_second = _bookmarks_so_far_this_second;
		_bookmarks_so_far_this_second = 0;
		_time_point_second_start += std::chrono::seconds(1);
			//std::chrono::duration < float, std::ratio<1, 1>>(1.0f);
	}

	return;
}

