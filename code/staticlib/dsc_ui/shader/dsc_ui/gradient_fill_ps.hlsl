#include "gradient_fill_interpolant.hlsli"

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer ConstantBuffer : register(b0)
{
	float _focus[4];
	float _colour_0[4];
	float _colour_1[4];
	float _colour_2[4];
	float _colour_3[4];
};

float4 CaculateWeight(float in_value)
{
	float4 result = float4(0.0, 0.0, 0.0, 0.0);
	if (in_value < _focus[0])
	{
		result[0] = 1.0f;
	}
	else if (in_value < _focus[1])
	{
		result[1] = (in_value - _focus[0]) / (_focus[1] - _focus[0]);
		result[0] = 1.0 - result[1];
	}
	else if (in_value < _focus[2])
	{
		result[2] = (in_value - _focus[1]) / (_focus[2] - _focus[1]);
		result[1] = 1.0 - result[2];
	}
	else if (in_value < _focus[3])
	{
		result[3] = (in_value - _focus[2]) / (_focus[3] - _focus[2]);
		result[2] = 1.0 - result[3];
	}
	else
	{
		result[3] = 1.0;
	}
	return result;
}

Pixel main(Interpolant in_input)
{
    Pixel result;

	float4 weight = CaculateWeight(in_input._uv[1]);

	//result._colour =
	//	(_colour_0 * weight[0]) +
	//	(_colour_1 * weight[1]) +
	//	(_colour_2 * weight[2]) +
	//	(_colour_3 * weight[3]);

	result._colour = float4(
		(_colour_0[0] * weight[0]) + (_colour_1[0] * weight[0]) + (_colour_2[0] * weight[0]) + (_colour_3[3] * weight[0]),
		(_colour_0[1] * weight[1]) + (_colour_1[1] * weight[1]) + (_colour_2[1] * weight[1]) + (_colour_3[3] * weight[1]),
		(_colour_0[2] * weight[2]) + (_colour_1[2] * weight[2]) + (_colour_2[2] * weight[2]) + (_colour_3[3] * weight[2]),
		(_colour_0[3] * weight[3]) + (_colour_1[3] * weight[3]) + (_colour_2[3] * weight[3]) + (_colour_3[3] * weight[3])
		);

    return result;
}