#include "scroll_interpolant.hlsli"

cbuffer ConstantBuffer1 : register(b1)
{
    float4 _tint;
    float4 _pixel_width_height;
    float4 _pixel_low_x_y_high_x_y;
};

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

float CalculateCoverageLow(float in_pixel_floor, float in_scroll_floor, float in_scroll_frac)
{
    float coverage = 0.0f;
    if (in_pixel_floor == in_scroll_floor)
    {
        coverage = 1.0 - in_scroll_frac;
    }
    else if (in_pixel_floor < in_scroll_floor)
    {
        coverage = 1.0;
    }
    return coverage;
}

float CalculateCoverageHigh(float in_pixel_floor, float in_scroll_floor, float in_scroll_frac)
{
    float coverage = 0.0f;
    if (in_pixel_floor == in_scroll_floor)
    {
        coverage = in_scroll_frac;
    }
    else if (in_pixel_floor < in_scroll_floor)
    {
        coverage = 1.0;
    }
    return coverage;
}
// so, my default trick of using a pixel wide rampp to generate aliasing, has the weakness of possibly sharing the alpha over two pixels for each edge
// what would alising look like if it was limited to only one pixel
//float value_x = max(0.0, ((in_ray_thickness - d) * _texture_size.x) + s_subpixel_thickness);
float CalculateCoverage(float2 in_uv)
{
    float2 pixel = in_uv * _pixel_width_height.xy;
    float2 pixel_floor = floor(pixel);
    float4 scroll_floor = floor(_pixel_low_x_y_high_x_y);
    float4 scroll_frac = frac(_pixel_low_x_y_high_x_y);
    float coverage_x = min(CalculateCoverageLow(pixel_floor.x, scroll_floor.x, scroll_frac.x), CalculateCoverageHigh(pixel_floor.x, scroll_floor.z, scroll_frac.z));
    float coverage_y = min(CalculateCoverageLow(pixel_floor.y, scroll_floor.y, scroll_frac.y), CalculateCoverageHigh(pixel_floor.y, scroll_floor.w, scroll_frac.w));
    float coverage = min(coverage_x, coverage_y);
    return coverage;
}

Pixel main(Interpolant in_input)
{
    Pixel result;
    float coverage = CalculateCoverage(in_input._uv);

    result._colour = _tint * coverage;
    //result._colour = float4(1.0, 0.0, 0.0, 1.0);
    //result._colour = _tint;

    return result;
}
