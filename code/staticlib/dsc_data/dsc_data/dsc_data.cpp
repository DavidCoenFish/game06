#include "dsc_data.h"

std::unique_ptr<DscData::JsonValue> DscData::MakeJsonObject()
{
	auto result = std::make_unique<DscData::JsonValue>();
	result->data.emplace<DscData::JsonObject>();
	return result;
}
