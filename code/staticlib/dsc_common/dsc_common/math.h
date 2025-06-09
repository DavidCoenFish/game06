#pragma once
#include "dsc_common.h"

#define DSC_PI 3.1415926535897932384626433832795f
#define DSC_PI_DIV_180 0.01745329251994329576923690768489f
#define DSC_180_DIV_PI 57.295779513082320876798154814105f

namespace DscCommon
{
class VectorFloat4;
namespace DscMath
{
	const int ScaleInt(const int in_value, const float in_scale);

//	/// 0xff000000 a:1.0f
//	/// 0x000000ff r:1.0f
//	const int ConvertColourToInt(const VectorFloat4& in_value);
//	/// map [0.0f ... 1.0f] -> [0 ... 255]
//	const int ConvertFloatToByte(const float in_value);
//
}
}