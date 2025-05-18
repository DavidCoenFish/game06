#include "common.h"
#include "file_overlay_local.h"

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
