#include "accessor.h"

const bool DscData::IsJsonNull(const std::unique_ptr<JsonValue>& in_value)
{
	if (nullptr != in_value)
	{
		return std::holds_alternative<std::monostate>(in_value->data);
	}
	return false;
}

const bool DscData::IsBool(const std::unique_ptr<JsonValue>& in_value)
{
	if (nullptr != in_value)
	{
		return std::holds_alternative<bool>(in_value->data);
	}
	return false;
}

const bool DscData::IsInt(const std::unique_ptr<JsonValue>& in_value)
{
	if (nullptr != in_value)
	{
		return std::holds_alternative<int32>(in_value->data);
	}
	return false;
}

const bool DscData::IsFloat(const std::unique_ptr<JsonValue>& in_value)
{
	if (nullptr != in_value)
	{
		return std::holds_alternative<float>(in_value->data);
	}
	return false;
}

const bool DscData::IsString(const std::unique_ptr<JsonValue>& in_value)
{
	if (nullptr != in_value)
	{
		return std::holds_alternative<std::string>(in_value->data);
	}
	return false;
}

const bool DscData::IsArray(const std::unique_ptr<JsonValue>& in_value)
{
	if (nullptr != in_value)
	{
		return std::holds_alternative<JsonArray>(in_value->data);
	}
	return false;
}

const bool DscData::IsObject(const std::unique_ptr<JsonValue>& in_value)
{
	if (nullptr != in_value)
	{
		return std::holds_alternative<JsonObject>(in_value->data);
	}
	return false;
}

const bool DscData::GetBool(const std::unique_ptr<JsonValue>& in_value)
{
	if (nullptr != in_value)
	{
		return std::get<bool>(in_value->data);
	}
	return false;
}

const int32 DscData::GetInt(const std::unique_ptr<JsonValue>& in_value)
{
	if (nullptr != in_value)
	{
		return std::get<int32>(in_value->data);
	}
	DSC_ASSERT_ALWAYS("accessing bad json node");
	return 0;
}

const float DscData::GetFloat(const std::unique_ptr<JsonValue>& in_value)
{
	if (nullptr != in_value)
	{
		return std::get<float>(in_value->data);
	}
	DSC_ASSERT_ALWAYS("accessing bad json node");
	return 0.0f;
}

const std::string& DscData::GetString(const std::unique_ptr<JsonValue>& in_value)
{
	if (nullptr != in_value)
	{
		return std::get<std::string>(in_value->data);
	}
	DSC_ASSERT_ALWAYS("accessing bad json node");
	static std::string s_dummy = {};
	return s_dummy;
}

const std::unique_ptr<DscData::JsonValue>& DscData::GetObjectChild(const JsonValue& in_parent, const std::string& in_key)
{
	const JsonObject& map_value = std::get<JsonObject>(in_parent.data);
	const auto iter = map_value.find(in_key);
	if (iter != map_value.end())
	{
		return iter->second;
	}

	static std::unique_ptr<JsonValue> s_dummy;
	return s_dummy;
}


const std::unique_ptr<DscData::JsonValue>& DscData::GetObjectChild(const std::unique_ptr<JsonValue>& in_parent, const std::string& in_key)
{
	if (nullptr != in_parent)
	{
		return GetObjectChild(*in_parent, in_key);
	}
	static std::unique_ptr<JsonValue> s_dummy;
	return s_dummy;
}

const int32 DscData::GetArrayCount(const std::unique_ptr<JsonValue>& in_parent)
{
	if (nullptr != in_parent)
	{
		const JsonArray& array_value = std::get<JsonArray>(in_parent->data);
		return static_cast<int32>(array_value.size());
	}
	return 0;
}

const std::unique_ptr<DscData::JsonValue>& DscData::GetArrayChild(const std::unique_ptr<JsonValue>& in_parent, const int32 in_index)
{
	if (nullptr != in_parent)
	{
		const JsonArray& array_value = std::get<JsonArray>(in_parent->data);
		DSC_ASSERT((0 <= in_index) && (in_index < static_cast<int32>(array_value.size())), "invalid param");
		return array_value[in_index];
	}
	static std::unique_ptr<JsonValue> s_dummy;
	return s_dummy;
}

// initial thought was to have DscData as read only, but could be conviemient to collect all data under one root
void DscData::SetObjectValue(std::unique_ptr<JsonValue>& in_parent, const std::string& in_key, std::unique_ptr<JsonValue>&& in_value)
{
	if (nullptr != in_parent)
	{
		JsonObject& map_value = std::get<JsonObject>(in_parent->data);
		map_value.emplace(in_key, std::move(in_value));
	}
	return;
}

