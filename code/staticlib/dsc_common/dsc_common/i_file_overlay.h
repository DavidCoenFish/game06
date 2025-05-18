#pragma once

#include "common.h"

namespace DscCommon
{
class IFileOverlay 
{
public:
	virtual ~IFileOverlay() {}

	virtual const bool LoadFile(std::vector<uint8>& out_data, const std::string& in_filePath) = 0;
	virtual const bool SaveFile(const std::vector<uint8>& in_data, const std::string& in_filePath) = 0;

}; // IFileOverlay
} //DscCommon
