#include "dsc_dag_group.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/file_system.h>
#include <dsc_common/i_file_overlay.h>
#include <dsc_common/log_system.h>
#include <dsc_data/dsc_data.h>
#include <dsc_data/json.h>
#include "test_util.h"

namespace
{
	const bool TestLoadFile()
	{
		DscCommon::LogSystem logSystem(DscCommon::LogLevel::Diagnostic);

		bool ok = true;
		DscCommon::FileSystem file_system;
		auto result = DscData::LoadJsonFromFile(
			file_system, 
			DscCommon::FileSystem::JoinPath("data", "sample_json", "test.json")
			);
		ok = TEST_UTIL_NOT_EQUAL(ok, (DscData::JsonValue*)nullptr, result.get());
		return ok;
	}
}//namespace

const bool TestDscData()
{
	bool ok = true;

	ok &= TestLoadFile();

	return ok;
}