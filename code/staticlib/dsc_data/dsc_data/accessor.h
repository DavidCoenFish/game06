#pragma once
#include "dsc_data.h"

namespace DscData
{
	const bool IsNull(const JsonValue& in_value);
	const bool IsBool(const JsonValue& in_value);
	const bool IsInt(const JsonValue& in_value);
	const bool IsFloat(const JsonValue& in_value);
	const bool IsString(const JsonValue& in_value);
	const bool IsArray(const JsonValue& in_value);
	const bool IsObject(const JsonValue& in_value);

	const bool SetObjectValue(JsonValue& in_parent, const std::string& in_key, std::unique_ptr<JsonValue>&& in_value);
	const bool AddArrayValue(JsonValue& in_parent, std::unique_ptr<JsonValue>&& in_value);
	const bool SetArrayValue(JsonValue& in_parent, const int32 in_index, std::unique_ptr<JsonValue>&& in_value);
	const bool GetArrayCount(JsonValue& in_parent);

	std::unique_ptr<JsonValue>& GetObjectChild(JsonValue& in_parent, const std::string& in_key);
	std::unique_ptr<JsonValue>& GetArrayChild(JsonValue& in_parent, const int32 in_index);

}

