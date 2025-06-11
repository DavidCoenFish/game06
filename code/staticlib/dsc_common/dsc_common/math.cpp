#include "math.h"
#include "vector_float4.h"

const int32 DscCommon::Math::ScaleInt(const int32 in_value, const float in_scale)
{
	return static_cast<int>(round(static_cast<float>(in_value) * in_scale));
}


const int32 DscCommon::Math::ConvertColourToInt(const uint8_t in_red, const uint8_t in_green, const uint8_t in_blue, const uint8_t in_alpha)
{
	const int32 result = in_red | 
		(in_green << 8) |
		(in_blue << 16) |
		(in_alpha << 24);
	return result;
}

const int32 DscCommon::Math::ConvertColourToInt(const VectorFloat4& in_value)
{
	const int32 result =
		ConvertFloatToByte(in_value.GetX()) |
		(ConvertFloatToByte(in_value.GetY()) << 8) |
		(ConvertFloatToByte(in_value.GetZ()) << 16) |
		(ConvertFloatToByte(in_value.GetW()) << 24);
	return result;
}

const int32 DscCommon::Math::ConvertFloatToByte(const float in_value)
{
	// mul by 256.0 and take the floor to get better mapping of range? 
	//example: converting a coin toss as expressed as [0... 1.0], you would multiply by 2 and floor
	const int32 result = std::max(0, std::min(255, static_cast<int>(floor(in_value * 256.0f))));
	return result;
}
