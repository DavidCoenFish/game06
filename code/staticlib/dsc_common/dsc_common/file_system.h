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

	FileSystem(std::unique_ptr<IFileOverlay>&& in_overlay = FactoryOverlayLocal());

	const int32 AddOverlay(std::unique_ptr<IFileOverlay>&& in_overlay);
	// try all overlay from the last added till one works
	const bool LoadFile(std::vector<uint8>& out_data, const std::string& in_filePath);
	const bool LoadFileOverlay(std::vector<uint8>& out_data, const std::string& in_filePath, const int32 in_overlayId);
	// try all overlay from the last added till one works
	const bool SaveFile(const std::vector<uint8>& in_data, const std::string& in_filePath);
	const bool SaveFileOverlay(const std::vector<uint8>& in_data, const std::string& in_filePath, const int32 in_overlayId);

	// query, issue is that this need some level of support for each level of the overlay, 
	// and even the possibly of only query one level of the overlay if it has file, is this modded file?
	// posibly have [DeleteFile, CreateFolder, DeleteFolder] 
	// less possible [QueryFile, ListFiles]

#if 0 //todo
	const int32 RequestAsyncLoad(const std::string& in_key);
	void HasAsyncLoadFinished(const int32 in_id);
	void HasAllAsyncLoadFinished(const int32 in_id);
#endif
private:
	std::vector<std::unique_ptr<IFileOverlay>> mOverlayArray = {};

}; //FileSystem
} //DscCommon