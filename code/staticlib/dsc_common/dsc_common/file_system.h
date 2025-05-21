#pragma once

#include "common.h"

namespace DscCommon
{
class IFileOverlay;

class FileSystem
{
public:
	// don't provide just a Load/Save local file as we may want to expressly prohibit this at certain stages of app development
	// the default overlay has a simple load/save file, and have that provided as default

	static const std::string GetTempFilePath();
	static const std::string JoinPath(const std::string& in_lhs, const std::string& in_rhs);
	static std::unique_ptr<IFileOverlay>&& FactoryOverlayLocal();

	FileSystem(std::vector<std::unique_ptr<IFileOverlay>>&& in_consumerArray);
	FileSystem(std::unique_ptr<IFileOverlay>&& in_overlay = FactoryOverlayLocal());

	const int32 AddOverlay(std::unique_ptr<IFileOverlay>&& in_overlay);
	// try all overlay from the last added till one works
	const bool LoadFile(std::vector<uint8>& out_data, const std::string& in_filePath);
	const bool LoadFileOverlay(std::vector<uint8>& out_data, const std::string& in_filePath, const int32 in_overlayId);
	// try all overlay from the last added till one works
	const bool SaveFile(const std::vector<uint8>& in_data, const std::string& in_filePath);
	const bool SaveFileOverlay(const std::vector<uint8>& in_data, const std::string& in_filePath, const int32 in_overlayId);

	const bool DeleteFile(const std::string& in_filePath);
	const bool DeleteFileOverlay(const std::string& in_filePath, const int32 in_overlayId);

	const bool CreateFolder(const std::string& in_filePath);
	const bool CreateFolderOverlay(const std::string& in_filePath, const int32 in_overlayId);

	const bool DeleteFolder(const std::string& in_filePath);
	const bool DeleteFolderOverlay(const std::string& in_filePath, const int32 in_overlayId);

	const bool QueryFile(const std::string& in_filePath);
	const bool QueryFileOverlay(const std::string& in_filePath, const int32 in_overlayId);

	const bool ListFiles(std::vector<std::string>& out_filePathArray, const std::string& in_filePath);
	const bool ListFilesOverlay(std::vector<std::string>& out_filePathArray, const std::string& in_filePath, const int32 in_overlayId);

#if 0 //todo
	const int32 RequestAsyncLoad(const std::string& in_key, const int32 in_overlayId);
	void HasAsyncLoadFinished(const int32 in_id);
	void HasAllAsyncLoadFinished(const int32 in_id);
#endif
private:
	std::vector<std::unique_ptr<IFileOverlay>> mOverlayArray = {};

}; //FileSystem
} //DscCommon