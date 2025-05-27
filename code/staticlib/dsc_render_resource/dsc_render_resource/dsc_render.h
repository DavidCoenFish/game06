#pragma once
#include <dsc_common/common.h>

#define LOG_TOPIC_DSC_RENDER "DSC_RENDER"

//#define DSC_LOG_RENDER_ERROR(FORMAT, ...)		DSC_LOG_ERROR(LOG_TOPIC_DSC_RENDER, FORMAT, ...)
//#define DSC_LOG_RENDER_WARNING(FORMAT, ...)		DSC_LOG_WARNING(LOG_TOPIC_DSC_RENDER, FORMAT, ...)
//#define DSC_LOG_RENDER_INFO(FORMAT, ...)		DSC_LOG_INFO(LOG_TOPIC_DSC_RENDER, FORMAT, ...)
//#define DSC_LOG_RENDER_DIAGNOSTIC(FORMAT, ...)	DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_RENDER, FORMAT, ...)

#ifndef IID_GRAPHICS_PPV_ARGS
#define IID_GRAPHICS_PPV_ARGS(x) IID_PPV_ARGS(x)
#endif

namespace DirectX
{
	/// Helpers for aligning values by a power of 2, round down
	template < typename T > inline T AlignDown(
		T in_size,
		size_t in_alignment
	) noexcept
	{
		if (in_alignment > 0)
		{
			assert(((in_alignment - 1) & in_alignment) == 0);
			auto mask = static_cast <T> (in_alignment - 1);
			return in_size & ~mask;
		}
		return in_size;
	}

	/// Helpers for aligning values by a power of 2, round up
	template < typename T > inline T AlignUp(
		T in_size,
		size_t in_alignment
	) noexcept
	{
		if (in_alignment > 0)
		{
			assert(((in_alignment - 1) & in_alignment) == 0);
			auto mask = static_cast <T> (in_alignment - 1);
			return (in_size + mask) & ~mask;
		}
		return in_size;
	}

	size_t BytesPerPixel(DXGI_FORMAT in_fmt);

	/// Helper class for COM exceptions
	class com_exception : public std::exception
	{
	public:
		/// Constructor
		com_exception(HRESULT hr) noexcept : result(hr) {}

		/// Get the descript of the exception
		const char* what() const override
		{
			static char s_str[64] = {};
			sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
			return s_str;
		}

	private:
		HRESULT result;
	};

	/// Helper utility converts D3D API failures into exceptions.
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw com_exception(hr);
		}
	}

} // DX
