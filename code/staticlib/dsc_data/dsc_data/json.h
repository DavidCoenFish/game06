#pragma once
#include "dsc_data.h"

namespace DscData
{
	//nlohmann/json or RapidJSON

	std::unique_ptr<JsonValue> LoadJsonFromFile(
		DscCommon::FileSystem& in_file_system,
		const std::string& in_file_path
	);

	void SaveJsonToFile(
		DscCommon::FileSystem& in_file_system,
		const std::string& in_file_path,
		const JsonValue& in_value
	);

}

