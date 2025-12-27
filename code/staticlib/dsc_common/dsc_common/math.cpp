#include "math.h"
#include "vector_float4.h"

const int32 DscCommon::Math::ScaleInt(const int32 in_value, const float in_scale)
{
	return static_cast<int>(round(static_cast<float>(in_value) * in_scale));
}

const uint32 DscCommon::Math::Ceiling(const uint32 in_value, const uint32 in_alignment)
{
	const uint32 pad = in_value % in_alignment;
	if (0 == pad)
	{
		return in_value;
	}
	return in_value + (in_alignment - pad);
}

const float DscCommon::Math::Clamp(const float in_value, const float in_low, const float in_high)
{
	// this order of operations is to convert (nan)in_value into in_low
	if ((in_low <= in_value) && (in_value <= in_high))
	{
		return in_value;
	}
	if (in_low < in_value)
	{
		return in_high;
	}

	return in_low;
}

// top left pixel [[0,0] ... [screen width,height]] => render bottom left [[-1.0,-1.0] ... [1.0, 1.0]]
const float DscCommon::Math::UIPixelsToRenderSpaceHorizontal(const int32 in_pixel, const int32 in_screen_width)
{
	return ((static_cast<float>(in_pixel) / static_cast<float>(in_screen_width)) * 2.0f) - 1.0f;
}

const float DscCommon::Math::UIPixelsToRenderSpaceVertical(const int32 in_pixel, const int32 in_screen_height)
{
	return ((static_cast<float>(in_pixel) / static_cast<float>(in_screen_height)) * -2.0f) + 1.0f;
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

const uint8 DscCommon::Math::ConvertFloatToByte(const float in_value)
{
	// mul by 256.0 and take the floor to get better mapping of range? 
	//example: converting a coin toss as expressed as [0... 1.0], you would multiply by 2 and floor
	const uint8 result = static_cast<uint8>(std::max(0, std::min(255, static_cast<int>(floor(in_value * 256.0f)))));
	return result;
}

const float DscCommon::Math::ConvertByteToFloat(const uint8 in_value)
{
	const float result = (static_cast<float>(in_value) / 256.0f);
	return result;
}

const bool DscCommon::Math::InsideBounds(const float in_x, const float in_y, const VectorFloat4& in_bounds)
{
	return ((in_bounds[0] < in_x) &&
		(in_bounds[1] < in_y) &&
		(in_x < in_bounds[2]) &&
		(in_y < in_bounds[3]));
}

// 3x^2 - 2x^3
const float DscCommon::Math::EaseInEaseOut(const float in_x)
{
	const float x2 = in_x * in_x;
	const float result = (3.0f * x2) - (2.0f * x2 * in_x);
	return result;
}

// fmod(-5, 3) gives -2, and we want 1, so we have PosFMod
const float DscCommon::Math::PosFMod(const float x, const float y)
{
	float result = fmod(x, y);
	if (result < 0.0f)
	{
		result += y;
	}
	return result;
}

