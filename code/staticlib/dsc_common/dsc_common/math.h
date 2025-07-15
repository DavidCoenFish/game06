#pragma once
#include "dsc_common.h"

#define DSC_PI 3.1415926535897932384626433832795f
#define DSC_PI_DIV_180 0.01745329251994329576923690768489f
#define DSC_180_DIV_PI 57.295779513082320876798154814105f

namespace DscCommon
{
	template <typename TYPE>
	class Vector4;
	typedef Vector4<float> VectorFloat4;

namespace Math
{
	const int32 ScaleInt(const int32 in_value, const float in_scale);
	const uint32 Ceiling(const uint32 in_value, const uint32 in_alignment);

	// top left pixel [[0,0] ... [screen width,height]] => render bottom left [[-1.0,-1.0] ... [1.0, 1.0]]
	const float UIPixelsToRenderSpaceHorizontal(const int32 in_pixel, const int32 in_screen_width);
	const float UIPixelsToRenderSpaceVertical(const int32 in_pixel, const int32 in_screen_height);

	constexpr int32 sWhite = 0xffffffff;
	constexpr int32 sRed = 0xff0000ff;

	/// 0xff000000 a:255
	/// 0x000000ff r:255
	const int32 ConvertColourToInt(const uint8_t in_red, const uint8_t in_green, const uint8_t in_blue, const uint8_t in_alpha);

	/// 0xff000000 a:1.0f
	/// 0x000000ff r:1.0f
	const int32 ConvertColourToInt(const VectorFloat4& in_value);
	/// map [0.0f ... 1.0f] -> [0 ... 255]
	const uint8 ConvertFloatToByte(const float in_value);
	/// map [0 ... 255] -> [0.0f ... 1.0f]
	const float ConvertByteToFloat(const uint8 in_value);

	/// since the function name is Inside, then only reurn true if x is bigger than low and smaller than high bounds
	const bool InsideBounds(const float in_x, const float in_y, const VectorFloat4& in_bounds);

}
}