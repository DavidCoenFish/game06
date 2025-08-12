#pragma once

#define LOG_TOPIC_DSC_DATA "DSC_DATA"

// allow the IDE to get definitions, otherwise the include paths are defined in the BFF script outside awarness of the IDE
// added DSC_BFF_BUILD to fastbuild defines
#ifndef DSC_BFF_BUILD

#include "..\..\dsc_common\dsc_common\dsc_common.h"

#pragma warning(push)
#pragma warning(disable : 4464)  //relative include path contains '..'
#ifdef GetObject
#undef GetObject
#endif
#include "..\..\..\sdk_include\rapidjson\document.h"
#pragma warning(pop)


#endif //#ifndef DSC_BFF_BUILD

#include <dsc_common/dsc_common.h>

namespace DscCommon
{
	class FileSystem;
}

/*
endgoal is to have a api like this, but with the data from a monolithic packed struct as to avoid allocations?
otherwise this looks like a lot of busywork replace the rapidjson::GenericObject<false, rapidjson::Value> with JsonValue
*/
namespace DscData
{
	//from google prompt> c++ std::variant std::unique json
	// Forward declarations for recursive types
	struct JsonValue;
	using JsonArray = std::vector<std::unique_ptr<JsonValue>>;
	using JsonObject = std::map<std::string, std::unique_ptr<JsonValue>>;

	// Define the variant for a single JSON value
	struct JsonValue {
		std::variant<
			std::monostate,	// Represents JSON null
			bool,
			int32,			// For integer numbers, could use int64 if needed
			float,			// For floating-point numbers
			std::string,
			JsonArray,
			JsonObject
		> data;
	};
}

