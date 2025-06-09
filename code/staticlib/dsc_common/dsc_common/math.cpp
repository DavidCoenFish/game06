#include "math.h"
#include "vector_float4.h"

const int DscCommon::DscMath::ScaleInt(const int in_value, const float in_scale)
{
	return static_cast<int>(round(static_cast<float>(in_value) * in_scale));
}


//const int DscCommon::DscMath::ConvertColourToInt(const VectorFloat4& in_value)
//{
//	const int result =
//		ConvertFloatToByte(in_value.GetX()) |
//		(ConvertFloatToByte(in_value.GetY()) << 8) |
//		(ConvertFloatToByte(in_value.GetZ()) << 16) |
//		(ConvertFloatToByte(in_value.GetW()) << 24);
//	return result;
//}
//
//const int DscCommon::DscMath::ConvertFloatToByte(const float in_value)
//{
//	// mul by 256.0 and take the floor to get better mapping of range? 
//	//example: converting a coin toss as expressed as [0... 1.0], you would multiply by 2 and floor
//	const int result = std::max(0, std::min(255, static_cast<int>(floor(in_value * 256.0f))));
//	return result;
//}
