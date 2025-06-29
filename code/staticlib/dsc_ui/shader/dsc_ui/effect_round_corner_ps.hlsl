#include "debug_grid_interpolant.hlsli"

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer ConstantBuffer : register(b0)
{
    float4 _width_height;
};

float CalculateCornerAlpha(
    float2 in_uv,
    float4 in_radius,
    float2 in_width_height,
)
{
    float2 pixel_pos = in_uv * in_width_height;

    //TRoundCorners, // data[bottom left, top left, Top right, bottom right] reference, margin is [left, top, right, bottom]

    float bottom_left = CalculateAlpha(
        max(0.0, in_radius.x - (pixel_pos.x - _margin.x) + 0.5),
        max(0.0, in_radius.x - (in_width_height.y - pixel_pos.y - _margin.w) + 0.5),
        in_radius.x
    );

    float top_left = CalculateAlpha(
        max(0.0, in_radius.y - (pixel_pos.x - _margin.x) + 0.5),
        max(0.0, in_radius.y - (pixel_pos.y - _margin.y) + 0.5),
        in_radius.y
    );

    float top_right = CalculateAlpha(
        max(0.0, in_radius.z - (in_width_height.x - pixel_pos.x - _margin.z) + 0.5),
        max(0.0, in_radius.z - (pixel_pos.y - _margin.y) + 0.5),
        in_radius.z
    );

    float bottom_right = CalculateAlpha(
        max(0.0, in_radius.w - (in_width_height.x - pixel_pos.x - _margin.z) + 0.5),
        max(0.0, in_radius.w - (in_width_height.y - pixel_pos.y - _margin.w) + 0.5),
        in_radius.w
    );

    return bottom_left * top_left * top_right * bottom_right;
}


Pixel main(Interpolant in_input)
{
    Pixel result;

    result._colour = texel * corner_alpha;

    return result;
}