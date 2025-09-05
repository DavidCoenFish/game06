#include "scroll_interpolant.hlsli"

cbuffer ConstantBuffer1 : register(b0)
{
	// colour of the knot
    float4 _tint;
	// render viewport size 
    float4 _pixel_width_height;
	// bounds of knot in pixels
    float4 _pixel_low_x_y_high_x_y;
};

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

float CalculateCoverage(float2 in_uv)
{
    float2 pixel = in_uv * _pixel_width_height.xy;

    float value_x_low = max(0.0, pixel.x - _pixel_low_x_y_high_x_y.x);
    float value_x_high = max(0.0, _pixel_low_x_y_high_x_y.z - pixel.x);
    float value_y_low = max(0.0, pixel.y - _pixel_low_x_y_high_x_y.y);
    float value_y_high = max(0.0, _pixel_low_x_y_high_x_y.w - pixel.y);

	float coverage = min(min(min(value_y_high, value_y_low), value_x_high), value_x_low);

    return saturate(coverage);
}

Pixel main(Interpolant in_input)
{
    Pixel result;
    float coverage = CalculateCoverage(in_input._uv);

    result._colour = _tint * coverage;
    //result._colour = float4(1.0, 0.0, 0.0, 1.0);
    //result._colour = _tint;
	//result._colour.x = in_input._uv.x;
	//result._colour.y = in_input._uv.y;

    return result;
}
