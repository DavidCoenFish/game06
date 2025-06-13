#include "dsc_png.h"
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/vector_int2.h>
#include <png.h>

namespace
{
	//https://pulsarengine.com/2009/01/reading-png-images-from-memory/
	class InputStream
	{
	public:
		InputStream() = delete;
		InputStream& operator=(const InputStream&) = delete;
		InputStream(const InputStream&) = delete;

		InputStream(std::vector<uint8>& in_png_file_data, const std::string& in_png_file_path)
			: _png_file_path(in_png_file_path)
			, _png_file_data(in_png_file_data)

		{
			// nop
		}
		size_t Read(byte* dest, const size_t byteCount)
		{
			size_t remaining = std::min(_png_file_data.size() - _index, byteCount);
			for (size_t index = 0; index < remaining; ++index)
			{
				dest[index] = _png_file_data[_index + index];
			}

			_index += remaining;
			return remaining;
		}

		const std::string& GetPngFilePath() const { return _png_file_path; }

	private:
		std::string _png_file_path;
		std::vector<uint8>& _png_file_data;
		size_t _index = 0;
	};

	void ReadDataFromInputStream(png_structp png_ptr, png_bytep outBytes,
		png_size_t byteCountToRead)
	{
		png_voidp io_ptr = png_get_io_ptr(png_ptr);
		if (io_ptr == NULL)
		{
			DSC_LOG_ERROR(LOG_TOPIC_DSC_PNG, "png read input failed\n");
			return;
		}

		InputStream* input_stream = reinterpret_cast<InputStream*>(io_ptr);
		const size_t bytesRead = input_stream->Read(
			(byte*)outBytes,
			(size_t)byteCountToRead);

		if ((png_size_t)bytesRead != byteCountToRead)
		{
			DSC_LOG_ERROR(LOG_TOPIC_DSC_PNG, "failed to read png:%s bytesRead:%d byteCountToRead:%d\n", input_stream->GetPngFilePath().c_str(), bytesRead, byteCountToRead);
			return;
		}
	}

	void ParsePngData(std::vector<uint8_t>& out_data, const png_uint_32 width, const png_uint_32 height, const int32 byte_per_pixel, const png_structp& png_ptr,
		const png_infop& info_ptr)
	{
		const size_t bytesPerRow = png_get_rowbytes(png_ptr, info_ptr);
		byte* rowData = new byte[bytesPerRow];
		int32 trace = 0;

		// read single row at a time
		for (uint32 rowIdx = 0; rowIdx < height; ++rowIdx)
		{
			png_read_row(png_ptr, (png_bytep)rowData, NULL);

			const uint32 rowOffset = rowIdx * width;

			uint32 byteIndex = 0;
			for (uint32 colIdx = 0; colIdx < width; ++colIdx)
			{
				for (int32 index = 0; index < byte_per_pixel; ++index)
				{
					out_data[trace++] = rowData[byteIndex++];
				}
			}
			DSC_ASSERT(byteIndex == bytesPerRow, "sanity check png bytesPerRow");
		}

		delete[] rowData;
	}


}

// failure logged, zero size data returned
void DscPng::LoadPng(
	std::vector<uint8_t>& out_data,
	int32_t& out_byte_per_pixel,
	DscCommon::VectorInt2& out_size,
	DscCommon::FileSystem& in_file_system,
	const std::string& in_file_path
	)
{
	out_data.clear();
	out_byte_per_pixel = 0;
	out_size.Set(0, 0);

	std::vector<uint8> png_file_data = {};
	if (false == in_file_system.LoadFile(png_file_data, in_file_path))
	{
		DSC_LOG_ERROR(LOG_TOPIC_DSC_PNG, "failed to load png:%s\n", in_file_path.c_str());
		return;
	}

	InputStream input_stream(png_file_data, in_file_path);

	enum { kPngSignatureLength = 8 };
	byte pngSignature[kPngSignatureLength];

	input_stream.Read(pngSignature, kPngSignatureLength);

	if (false == png_check_sig(pngSignature, kPngSignatureLength))
	{
		DSC_LOG_ERROR(LOG_TOPIC_DSC_PNG, "png sig failed, png:%s\n", in_file_path.c_str());
		return;
	}

	png_structp png_ptr = NULL;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		DSC_LOG_ERROR(LOG_TOPIC_DSC_PNG, "png read struct failed, png:%s\n", in_file_path.c_str());
		return;
	}

	// get PNG image data info struct (memory is allocated by libpng)
	png_infop info_ptr = NULL;
	info_ptr = png_create_info_struct(png_ptr);

	if (info_ptr == NULL)
	{
		DSC_LOG_ERROR(LOG_TOPIC_DSC_PNG, "png info struct failed, png:%s\n", in_file_path.c_str());
		// libpng must free file info struct memory before we bail
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return;
	}

	png_set_read_fn(png_ptr, &input_stream, ReadDataFromInputStream);

	// tell libpng we already read the signature
	png_set_sig_bytes(png_ptr, kPngSignatureLength);

	png_read_info(png_ptr, info_ptr);

	png_uint_32 width = 0;
	png_uint_32 height = 0;
	int bitDepth = 0;// bitDepth is bits per channel
	int colorType = -1;
	png_uint_32 retval = png_get_IHDR(png_ptr, info_ptr,
		&width,
		&height,
		&bitDepth,
		&colorType,
		NULL, NULL, NULL);

	if (retval != 1)
	{
		DSC_LOG_ERROR(LOG_TOPIC_DSC_PNG, "png ihdr failed, png:%s retval:%d\n", in_file_path.c_str(), retval);
		// libpng must free file info struct memory before we bail
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return;
	}

	out_size.Set(width, height);

	switch (colorType)
	{
	case PNG_COLOR_TYPE_RGB:
		out_byte_per_pixel = 3;
		break;

	case PNG_COLOR_TYPE_RGB_ALPHA:
		out_byte_per_pixel = 4;
		break;

	default:
		DSC_LOG_ERROR(LOG_TOPIC_DSC_PNG, "bad colour type, png:%s colorType:%d\n", in_file_path.c_str(), colorType);
		// libpng must free file info struct memory before we bail
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return;
	}

	out_data.resize(width * height * out_byte_per_pixel);

	ParsePngData(out_data, width, height, out_byte_per_pixel, png_ptr, info_ptr);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	return;
}

#if 0
void DscPng::SaveImage(
	const std::vector<uint8_t>& in_data,
	const int32_t in_byte_per_pixel,
	DscCommon::VectorInt2& in_size,
	DscCommon::FileSystem& in_file_system,
	const std::string& in_file_path
	)
{
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_init_io(png_ptr, fp);
	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}
#endif