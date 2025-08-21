#include "json.h"
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>

#pragma warning(push)
#pragma warning(disable : 4464)  //relative include path contains '..'
#ifdef GetObject
#undef GetObject
#endif
#include <rapidjson/document.h>
#pragma warning(pop)

namespace
{
	std::unique_ptr<DscData::JsonValue> LoadJsonObject(
		const rapidjson::GenericObject<true, rapidjson::Value>& in_object
		DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_file_path)
	);

	std::unique_ptr<DscData::JsonValue> LoadJson(
		const rapidjson::GenericObject<true, rapidjson::Value>::PlainType& in_object
		DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_file_path)
	)
	{
		switch (in_object.GetType())
		{
		default:
			DSC_LOG_ERROR(LOG_TOPIC_DSC_DATA, "Missing json type case:%d\n", static_cast<int8>(in_object.GetType()));
			break;
		case rapidjson::Type::kNullType:
		{
			auto value = std::make_unique<DscData::JsonValue>();
			return value;
		}
		case rapidjson::Type::kFalseType:
		{
			auto value = std::make_unique<DscData::JsonValue>();
			value->data = false;
			return value;
		}
		case rapidjson::Type::kTrueType:
		{
			auto value = std::make_unique<DscData::JsonValue>();
			value->data = true;
			return value;
		}
		case rapidjson::Type::kObjectType:
		{
			auto value = LoadJsonObject(
				in_object.GetObject()
				DSC_DEBUG_ONLY(DSC_COMMA in_file_path)
			);
			return value;
		}
		case rapidjson::Type::kArrayType:
		{
			auto value = std::make_unique<DscData::JsonValue>();
			value->data.emplace<DscData::JsonArray>();
			auto& array_data = std::get<DscData::JsonArray>(value->data);

			for (const auto& child : in_object.GetArray())
			{
				auto child_value = LoadJson(
					child
					DSC_DEBUG_ONLY(DSC_COMMA in_file_path)
				);
				array_data.push_back(std::move(child_value));
			}
			return value;
		}
		case rapidjson::Type::kStringType:
		{
			auto value = std::make_unique<DscData::JsonValue>();
			value->data.emplace<std::string>(std::string(in_object.GetString()));
			return value;
		}
		break;
		case rapidjson::Type::kNumberType:
		{
			auto value = std::make_unique<DscData::JsonValue>();
			if (true == in_object.IsInt())
			{
				value->data.emplace<int32>(in_object.GetInt());
			}
			else if (true == in_object.IsFloat())
			{
				value->data.emplace<float>(in_object.GetFloat());
			}
			else
			{
				DSC_LOG_ERROR(LOG_TOPIC_DSC_DATA, "Json number type out of currently supported range:%s\n", in_file_path.c_str());
			}

			return value;
		}
		break;

		}
		return nullptr;
	}

	std::unique_ptr<DscData::JsonValue> LoadJsonObject(
		const rapidjson::GenericObject<true, rapidjson::Value>& in_object
		DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_file_path)
		)
	{
		auto result = std::make_unique<DscData::JsonValue>();
		result->data.emplace<DscData::JsonObject>();
		auto& map_data = std::get<DscData::JsonObject>(result->data);

		for (const auto& iter : in_object)
		{
			//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DATA, "load object key:%s type:%d\n", iter.name.GetString(), static_cast<int8>(iter.value.GetType()));
			auto value = LoadJson(
				iter.value
				DSC_DEBUG_ONLY(DSC_COMMA in_file_path)
			);
			if (nullptr != value)
			{
				map_data[iter.name.GetString()] = std::move(value);
			}
		}

		return result;
	}
}

std::unique_ptr<DscData::JsonValue> DscData::LoadJsonFromFile(
	DscCommon::FileSystem& in_file_system,
	const std::string& in_file_path
	)
{
	std::vector<uint8> file_data = {};
	if (true != in_file_system.LoadFile(file_data, in_file_path))
	{
		DSC_LOG_ERROR(LOG_TOPIC_DSC_DATA, "File not found for json load:%s\n", in_file_path.c_str());
		return nullptr;
	}

	rapidjson::Document document;
	static_assert(sizeof(char) == sizeof(uint8), "assuming size of char");
	document.Parse(reinterpret_cast<const char*>(file_data.data()), file_data.size());

	if (false == document.IsObject())
	{
		DSC_LOG_ERROR(LOG_TOPIC_DSC_DATA, "Json document not an object:%s\n", in_file_path.c_str());
		return nullptr;
	}

	const rapidjson::Document& const_document = document;
	auto result = LoadJsonObject(
		const_document.GetObject()
		DSC_DEBUG_ONLY(DSC_COMMA in_file_path)
		);

	return std::move(result);
}

