#include <dsc_common\common.h>
#include <dsc_common\file_system.h>
#include <dsc_common\file_overlay_local.h>
#include "dsc_common_file_system.h"
#include "test_util.h"

namespace
{
const bool TestTempPath()
{
	const std::string tempPath = DscCommon::FileSystem::GetTempFilePath();
	bool ok = true;
	ok = TEST_UTIL_EQUAL(ok, tempPath.empty(), false);
	return ok;
}

const bool TestPathJoin()
{
	const std::string tempPath = DscCommon::FileSystem::GetTempFilePath();
	bool ok = true;
	ok = TEST_UTIL_EQUAL(ok, std::string("a\\b"), DscCommon::FileSystem::JoinPath("a", "b"));
	ok = TEST_UTIL_EQUAL(ok, std::string("a\\b\\c"), DscCommon::FileSystem::JoinPath("a\\b", "c"));
	ok = TEST_UTIL_EQUAL(ok, std::string("a\\b"), DscCommon::FileSystem::JoinPath("a\\", "b"));
	return ok;
}

const bool TestPathLocalFileSystemBasic()
{
	const std::string tempPath = DscCommon::FileSystem::GetTempFilePath();
	DscCommon::FileSystem fileSystem;
	const std::string filePath = DscCommon::FileSystem::JoinPath(tempPath, "test.txt");
	const std::vector<uint8> testData({0,1,2,3,4,5,6,7});

	bool ok = true;

	// inital delete is allowed to fail
	fileSystem.DeleteFile(filePath);
	ok = TEST_UTIL_EQUAL(ok, fileSystem.SaveFile(testData, filePath), true);
	ok = TEST_UTIL_EQUAL(ok, fileSystem.QueryFile(filePath), true);

	std::vector<uint8> testLoadData = {};
	ok = TEST_UTIL_EQUAL(ok, fileSystem.LoadFile(testLoadData, filePath), true);
	ok = TEST_UTIL_EQUAL(ok, fileSystem.DeleteFile(filePath), true);
	ok = TEST_UTIL_EQUAL(ok, fileSystem.QueryFile(filePath), false);

	ok = TEST_UTIL_EQUAL_VECTOR(ok, testData, testLoadData);

	return ok;
}

}//namespace

const bool DscCommonFileSystem()
{
	bool ok = true;

	ok &= TestTempPath();
	ok &= TestPathJoin();
	ok &= TestPathLocalFileSystemBasic();

	return ok;
}