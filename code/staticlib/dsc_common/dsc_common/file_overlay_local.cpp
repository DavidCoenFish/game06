#include "common.h"
#include "file_overlay_local.h"


DscCommon::FileOverlayLocal::FileOverlayLocal()
{
	//nop
}

DscCommon::FileOverlayLocal::~FileOverlayLocal()
{
	//nop
}

const bool DscCommon::FileOverlayLocal::LoadFile(std::vector<uint8>& out_data, const std::string& in_filePath)
{
	std::ifstream stream;
	stream.open(in_filePath, std::ios_base::binary | std::ios_base::in);
	if (!stream.good())
	{
		return false;
	}

    stream.seekg (0, stream.end);
    const auto length = stream.tellg();
    stream.seekg (0, stream.beg);

	out_data.resize(length);
	stream.read(reinterpret_cast<char*>(out_data.data()), length);
	stream.close();
	return true;
}

const bool DscCommon::FileOverlayLocal::SaveFile(const std::vector<uint8>& in_data, const std::string& in_filePath)
{
	std::ofstream stream;
	stream.open(in_filePath, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
	if (!stream.good())
	{
		return false;
	}

	stream.write(reinterpret_cast<const char*>(in_data.data()), in_data.size());

	stream.close();
	return true;
}

const bool DscCommon::FileOverlayLocal::DeleteFile(const std::string& in_filePath)
{
	return std::filesystem::remove(in_filePath);
}

const bool DscCommon::FileOverlayLocal::CreateFolder(const std::string& in_filePath)
{
	return std::filesystem::create_directory(in_filePath);
}

const bool DscCommon::FileOverlayLocal::DeleteFolder(const std::string& in_filePath)
{
	return (0 < std::filesystem::remove_all(in_filePath));
}

const bool DscCommon::FileOverlayLocal::QueryFile(const std::string& in_filePath)
{
	return std::filesystem::exists(in_filePath);
}

const bool DscCommon::FileOverlayLocal::ListFiles(std::vector<std::string>& out_filePathArray, const std::string& in_filePath)
{
	for (const auto& entry : std::filesystem::directory_iterator(in_filePath))
	{
		out_filePathArray.push_back(entry.path().string());
	}
	return (0 < out_filePathArray.size());
}

