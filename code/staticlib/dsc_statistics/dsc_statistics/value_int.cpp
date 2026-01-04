#include "value_int.h"

std::unique_ptr<DscStatistics::ValueInt> DscStatistics::ValueInt::Factory(
	const std::string& in_name,
	const int32 in_value
)
{
	return std::make_unique<ValueInt>(in_name, in_value);
}

DscStatistics::ValueInt::ValueInt(
	const std::string& in_name,
	const int32 in_value
)
	: _name(in_name)
	, _value(in_value)
{
	return;
}

const std::string DscStatistics::ValueInt::GetDescription() const
{
	return _name + std::string(": ") + std::to_string(_value);
}
