#include "common.h"
#include "i_file_overlay.h"

namespace DscCommon
{
class FileOverlayLocal : public IFileOverlay
{
public:
	FileOverlayLocal();
	~FileOverlayLocal();

	virtual const bool LoadFile(std::vector<uint8>& out_data, const std::string& in_filePath) override;
	virtual const bool SaveFile(const std::vector<uint8>& in_data, const std::string& in_filePath) override;
	virtual const bool DeleteFile(const std::string& in_filePath) override;
	virtual const bool CreateFolder(const std::string& in_filePath) override;
	virtual const bool DeleteFolder(const std::string& in_filePath) override;
	virtual const bool QueryFile(const std::string& in_filePath) override;
	virtual const bool ListFiles(std::vector<std::string>& out_filePathArray, const std::string& in_filePath) override;

}; // IFileOverlay
} //DscCommon
