#include "bookmark.h"

std::unique_ptr<DscStatistics::Bookmark> DscStatistics::Bookmark::Factory(
	const std::string& in_name,
	const std::chrono::system_clock::time_point& in_time_point
)
{
	return std::make_unique<DscStatistics::Bookmark>(in_name, in_time_point);
}

DscStatistics::Bookmark::Bookmark(
	const std::string& in_name,
	const std::chrono::system_clock::time_point& in_time_point
)
	: _name(in_name)
	, _time_point(in_time_point)
{
	//nop
}

const std::string DscStatistics::Bookmark::GetDescription() const
{
	//C++20
	//return _name + std::string(": ") + std::format("{}", _time_point);

	std::time_t tt = std::chrono::system_clock::to_time_t(_time_point);
	//std::tm tm = *std::gmtime(&tt);
	std::tm tm;
	gmtime_s(&tm, &tt);
	std::stringstream ss;
	ss << _name << ": " << std::put_time(&tm, "%H:%M:%S");
	return ss.str();
}

