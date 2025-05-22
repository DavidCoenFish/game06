#include "common.h"
#include "file_system.h"
#include "file_overlay_local.h"

namespace
{
	//std::unique_ptr<DscCommon::IFileOverlay> TestCreation()
	//{
	//	return std::make_unique<DscCommon::FileOverlayLocal>();
	//}
} //namespace

const std::string DscCommon::FileSystem::GetTempFilePath()
{
	return std::filesystem::temp_directory_path().string(); // u8string();
}

const std::string DscCommon::FileSystem::JoinPath(const std::string& in_lhs, const std::string& in_rhs)
{
	return (std::filesystem::path(in_lhs) / in_rhs).string(); //.u8string();
}

std::unique_ptr<DscCommon::IFileOverlay> DscCommon::FileSystem::FactoryOverlayLocal()
{
	//auto pConsumer = std::make_unique<FileOverlayLocal>();
	//return std::move(pConsumer);
	//return std::make_unique<FileOverlayLocal>();
	//return TestCreation();
	return std::make_unique<FileOverlayLocal>();
}

DscCommon::FileSystem::FileSystem(std::vector<std::unique_ptr<IFileOverlay>>&& in_consumerArray)
	: mOverlayArray(std::move(in_consumerArray))
{
	// nop
}

DscCommon::FileSystem::FileSystem(std::unique_ptr<IFileOverlay>&& in_overlay)
{
	AddOverlay(std::move(in_overlay));
}

const int32 DscCommon::FileSystem::AddOverlay(std::unique_ptr<IFileOverlay>&& in_overlay)
{
	const int32 index = static_cast<int32>(mOverlayArray.size());
	mOverlayArray.push_back(std::move(in_overlay));
	return index;
}

const bool DscCommon::FileSystem::LoadFile(std::vector<uint8>& out_data, const std::string& in_filePath)
{
	for (std::vector<std::unique_ptr<IFileOverlay>>::reverse_iterator riter = mOverlayArray.rbegin();
		riter != mOverlayArray.rend(); ++riter)
	{
		if (true == (*riter)->LoadFile(out_data, in_filePath))
		{
			return true;
		}
	}
	return false;
}

const bool DscCommon::FileSystem::LoadFileOverlay(std::vector<uint8>& out_data, const std::string& in_filePath, const int32 in_overlayId)
{
	if ((0 <= in_overlayId) && (in_overlayId <= static_cast<int32>(mOverlayArray.size())))
	{
		return mOverlayArray[in_overlayId]->LoadFile(out_data, in_filePath);
	}
	return false;
}

const bool DscCommon::FileSystem::SaveFile(const std::vector<uint8>& in_data, const std::string& in_filePath)
{
	for (std::vector<std::unique_ptr<IFileOverlay>>::reverse_iterator riter = mOverlayArray.rbegin();
		riter != mOverlayArray.rend(); ++riter)
	{
		if (true == (*riter)->SaveFile(in_data, in_filePath))
		{
			return true;
		}
	}
	return false;
}

const bool DscCommon::FileSystem::SaveFileOverlay(const std::vector<uint8>& in_data, const std::string& in_filePath, const int32 in_overlayId)
{
	if ((0 <= in_overlayId) && (in_overlayId <= static_cast<int32>(mOverlayArray.size())))
	{
		return mOverlayArray[in_overlayId]->SaveFile(in_data, in_filePath);
	}
	return false;
}

const bool DscCommon::FileSystem::DeleteFile(const std::string& in_filePath)
{
	for (std::vector<std::unique_ptr<IFileOverlay>>::reverse_iterator riter = mOverlayArray.rbegin();
		riter != mOverlayArray.rend(); ++riter)
	{
		if (true == (*riter)->DeleteFile(in_filePath))
		{
			return true;
		}
	}
	return false;
}

const bool DscCommon::FileSystem::DeleteFileOverlay(const std::string& in_filePath, const int32 in_overlayId)
{
	if ((0 <= in_overlayId) && (in_overlayId <= static_cast<int32>(mOverlayArray.size())))
	{
		return mOverlayArray[in_overlayId]->DeleteFile(in_filePath);
	}
	return false;
}

const bool DscCommon::FileSystem::CreateFolder(const std::string& in_filePath)
{
	for (std::vector<std::unique_ptr<IFileOverlay>>::reverse_iterator riter = mOverlayArray.rbegin();
		riter != mOverlayArray.rend(); ++riter)
	{
		if (true == (*riter)->CreateFolder(in_filePath))
		{
			return true;
		}
	}
	return false;
}

const bool DscCommon::FileSystem::CreateFolderOverlay(const std::string& in_filePath, const int32 in_overlayId)
{
	if ((0 <= in_overlayId) && (in_overlayId <= static_cast<int32>(mOverlayArray.size())))
	{
		return mOverlayArray[in_overlayId]->CreateFolder(in_filePath);
	}
	return false;
}

const bool DscCommon::FileSystem::DeleteFolder(const std::string& in_filePath)
{
	for (std::vector<std::unique_ptr<IFileOverlay>>::reverse_iterator riter = mOverlayArray.rbegin();
		riter != mOverlayArray.rend(); ++riter)
	{
		if (true == (*riter)->DeleteFolder(in_filePath))
		{
			return true;
		}
	}
	return false;
}

const bool DscCommon::FileSystem::DeleteFolderOverlay(const std::string& in_filePath, const int32 in_overlayId)
{
	if ((0 <= in_overlayId) && (in_overlayId <= static_cast<int32>(mOverlayArray.size())))
	{
		return mOverlayArray[in_overlayId]->DeleteFolder(in_filePath);
	}
	return false;
}

const bool DscCommon::FileSystem::QueryFile(const std::string& in_filePath)
{
	for (std::vector<std::unique_ptr<IFileOverlay>>::reverse_iterator riter = mOverlayArray.rbegin();
		riter != mOverlayArray.rend(); ++riter)
	{
		if (true == (*riter)->QueryFile(in_filePath))
		{
			return true;
		}
	}
	return false;
}

const bool DscCommon::FileSystem::QueryFileOverlay(const std::string& in_filePath, const int32 in_overlayId)
{
	if ((0 <= in_overlayId) && (in_overlayId <= static_cast<int32>(mOverlayArray.size())))
	{
		return mOverlayArray[in_overlayId]->QueryFile(in_filePath);
	}
	return false;
}

const bool DscCommon::FileSystem::ListFiles(std::vector<std::string>& out_filePathArray, const std::string& in_filePath)
{
	for (std::vector<std::unique_ptr<IFileOverlay>>::reverse_iterator riter = mOverlayArray.rbegin();
		riter != mOverlayArray.rend(); ++riter)
	{
		if (true == (*riter)->ListFiles(out_filePathArray, in_filePath))
		{
			return true;
		}
	}
	return false;
}

const bool DscCommon::FileSystem::ListFilesOverlay(std::vector<std::string>& out_filePathArray, const std::string& in_filePath, const int32 in_overlayId)
{
	if ((0 <= in_overlayId) && (in_overlayId <= static_cast<int32>(mOverlayArray.size())))
	{
		return mOverlayArray[in_overlayId]->ListFiles(out_filePathArray, in_filePath);
	}
	return false;
}
