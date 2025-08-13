#include "dsc_dag_group.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/file_system.h>
#include <dsc_common/i_file_overlay.h>
#include <dsc_common/log_system.h>
#include <dsc_data/dsc_data.h>
#include <dsc_data/json.h>
#include <dsc_data/accessor.h>
#include "test_util.h"

namespace
{
	const bool TestLoadFile()
	{
		DscCommon::LogSystem logSystem(DscCommon::LogLevel::Diagnostic);

		bool ok = true;
		DscCommon::FileSystem file_system;
		auto json_data = DscData::LoadJsonFromFile(
			file_system, 
			DscCommon::FileSystem::JoinPath("data", "sample_json", "test.json")
			);
		ok = TEST_UTIL_NOT_EQUAL(ok, (DscData::JsonValue*)nullptr, json_data.get());

		ok = TEST_UTIL_EQUAL(ok, true, DscData::IsString(DscData::GetObjectChild(json_data, "s")));
		ok = TEST_UTIL_EQUAL_STRING(ok, "hello world", DscData::GetString(DscData::GetObjectChild(json_data, "s")));
		ok = TEST_UTIL_EQUAL(ok, true, DscData::IsBool(DscData::GetObjectChild(json_data, "t")));
		ok = TEST_UTIL_EQUAL(ok, true, DscData::GetBool(DscData::GetObjectChild(json_data, "t")));
		ok = TEST_UTIL_EQUAL(ok, true, DscData::IsBool(DscData::GetObjectChild(json_data, "f")));
		ok = TEST_UTIL_EQUAL(ok, false, DscData::GetBool(DscData::GetObjectChild(json_data, "f")));
		ok = TEST_UTIL_EQUAL(ok, true, DscData::IsJsonNull(DscData::GetObjectChild(json_data, "n")));
		ok = TEST_UTIL_EQUAL(ok, true, DscData::IsInt(DscData::GetObjectChild(json_data, "i")));
		ok = TEST_UTIL_EQUAL(ok, 123, DscData::GetInt(DscData::GetObjectChild(json_data, "i")));

		ok = TEST_UTIL_EQUAL(ok, true, DscData::IsFloat(DscData::GetObjectChild(json_data, "pi")));
		ok = TEST_UTIL_ALMOST_EQUAL(ok, 3.1416f, DscData::GetFloat(DscData::GetObjectChild(json_data, "pi")));

		ok = TEST_UTIL_EQUAL(ok, true, DscData::IsArray(DscData::GetObjectChild(json_data, "a")));
		ok = TEST_UTIL_EQUAL(ok, 4, DscData::GetArrayCount(DscData::GetObjectChild(json_data, "a")));
		{
			const auto& json_array = DscData::GetObjectChild(json_data, "a");
			ok = TEST_UTIL_EQUAL(ok, 1, DscData::GetInt(DscData::GetArrayChild(json_array, 0)));
			ok = TEST_UTIL_EQUAL(ok, 2, DscData::GetInt(DscData::GetArrayChild(json_array, 1)));
			ok = TEST_UTIL_EQUAL(ok, 3, DscData::GetInt(DscData::GetArrayChild(json_array, 2)));
			ok = TEST_UTIL_EQUAL(ok, 4, DscData::GetInt(DscData::GetArrayChild(json_array, 3)));
		}

		ok = TEST_UTIL_EQUAL(ok, true, DscData::IsObject(DscData::GetObjectChild(json_data, "o")));
		{
			const auto& json_object = DscData::GetObjectChild(json_data, "o");
			ok = TEST_UTIL_EQUAL(ok, 456, DscData::GetInt(DscData::GetObjectChild(json_object, "ci")));
			ok = TEST_UTIL_EQUAL(ok, true, DscData::GetBool(DscData::GetObjectChild(json_object, "ct")));
		}

		return ok;
	}
}//namespace

const bool TestDscData()
{
	bool ok = true;

	ok &= TestLoadFile();

	return ok;
}