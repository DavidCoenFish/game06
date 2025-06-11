#pragma once
#include "dsc_common.h"

#define DSC_PI 3.1415926535897932384626433832795f
#define DSC_PI_DIV_180 0.01745329251994329576923690768489f
#define DSC_180_DIV_PI 57.295779513082320876798154814105f

namespace DscCommon
{
class VectorFloat4;
namespace Math
{
	const int32 ScaleInt(const int32 in_value, const float in_scale);

	constexpr int32 sWhite = 0xffffffff;
	constexpr int32 sRed = 0xff0000ff;

	/// 0xff000000 a:255
	/// 0x000000ff r:255
	const int32 ConvertColourToInt(const uint8_t in_red, const uint8_t in_green, const uint8_t in_blue, const uint8_t in_alpha);

	/// 0xff000000 a:1.0f
	/// 0x000000ff r:1.0f
	const int32 ConvertColourToInt(const VectorFloat4& in_value);
	/// map [0.0f ... 1.0f] -> [0 ... 255]
	const int32 ConvertFloatToByte(const float in_value);

}
}