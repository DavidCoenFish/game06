#include "main.h"

#include <dsc_common/file_system.h>
#include <dsc_common/i_file_overlay.h>
#include <dsc_common/log_system.h>
#include <dsc_common/math.h>
#include <dsc_common/vector_int2.h>
#include <dsc_png/dsc_png.h>

namespace
{
	void PremultiplePixelData4(std::vector<uint8_t>& in_data)
	{
		//float cumulative_error[3] = {};
		for (int32 index = 0; index < static_cast<int32>(in_data.size()); index += 4)
		{
			const float red = DscCommon::Math::ConvertByteToFloat(in_data[index + 0]);
			const float green = DscCommon::Math::ConvertByteToFloat(in_data[index + 1]);
			const float blue = DscCommon::Math::ConvertByteToFloat(in_data[index + 2]);
			const float alpha = DscCommon::Math::ConvertByteToFloat(in_data[index + 3]);

			in_data[index + 0] = DscCommon::Math::ConvertFloatToByte(red * alpha);
			in_data[index + 1] = DscCommon::Math::ConvertFloatToByte(green * alpha);
			in_data[index + 2] = DscCommon::Math::ConvertFloatToByte(blue * alpha);
		}
	}
}

//int main(int argc, char* argv[], char* envp[])
int main(int argc, char* argv[], char*)
{
    DscCommon::LogSystem log_system(DscCommon::LogLevel::Diagnostic);
	DscCommon::FileSystem file_system;

	for (int32 index = 1; index < argc; ++index)
	{
		std::string path(argv[index]);
		DSC_LOG_DIAGNOSTIC(LOG_TOPIC_APPLICATION, "argv:%s", path.c_str());

		std::vector<uint8_t> data = {};
		int32_t byte_per_pixel = 0;
		DscCommon::VectorInt2 size;
		DscPng::LoadPng(data, byte_per_pixel, size, file_system, path);
		if (data.size() <= 0)
		{
			DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "no pixeldata loaded:%s", path.c_str());
			continue;
		}

		if (4 == byte_per_pixel)
		{
			PremultiplePixelData4(data);

			DscPng::SavePng(data, byte_per_pixel, size, file_system, path + "_out.png");
		}
	}

	return 0;
}

