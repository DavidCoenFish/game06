#pragma once
#include "dsc_data.h"

namespace DscData
{
	// is it a valid json null marker, not just a null ptr
	const bool IsJsonNull(const std::unique_ptr<JsonValue>& in_value);
	const bool IsBool(const std::unique_ptr<JsonValue>& in_value);
	const bool IsInt(const std::unique_ptr<JsonValue>& in_value);
	const bool IsFloat(const std::unique_ptr<JsonValue>& in_value);
	const bool IsString(const std::unique_ptr<JsonValue>& in_value);
	const bool IsArray(const std::unique_ptr<JsonValue>& in_value);
	const bool IsObject(const std::unique_ptr<JsonValue>& in_value);

	const bool GetBool(const std::unique_ptr<JsonValue>& in_value);
	const int32 GetInt(const std::unique_ptr<JsonValue>& in_value);
	const float GetFloat(const std::unique_ptr<JsonValue>& in_value);
	const std::string& GetString(const std::unique_ptr<JsonValue>& in_value);

	const std::unique_ptr<JsonValue>& GetObjectChild(const JsonValue& in_parent, const std::string& in_key);
	const std::unique_ptr<JsonValue>& GetObjectChild(const std::unique_ptr<JsonValue>& in_parent, const std::string& in_key);
	const int32 GetArrayCount(const std::unique_ptr<JsonValue>& in_parent);
	const std::unique_ptr<JsonValue>& GetArrayChild(const std::unique_ptr<JsonValue>& in_parent, const int32 in_index);

	// initial thought was to have DscData as read only, but could be conviemient to collect all data under one root
	void SetObjectValue(std::unique_ptr<JsonValue>& in_parent, const std::string& in_key, std::unique_ptr<JsonValue>&& in_value);
	//const bool AddArrayValue(std::unique_ptr<JsonValue>& in_parent, std::unique_ptr<JsonValue>&& in_value);
	//const bool SetArrayValue(std::unique_ptr<JsonValue>& in_parent, const int32 in_index, std::unique_ptr<JsonValue>&& in_value);

}

