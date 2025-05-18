#include "common.h"
#include "i_file_overlay.h"

namespace DscCommon
{
class FileOverlayLocal : public IFileOverlay
{
public:
	virtual const bool LoadFile(std::vector<uint8>& out_data, const std::string& in_filePath) override;
	virtual const bool SaveFile(const std::vector<uint8>& in_data, const std::string& in_filePath) override;

}; // IFileOverlay
} //DscCommon
