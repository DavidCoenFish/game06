#include "effect_round_corner_interpolant.hlsli"

Texture2D g_texture : register(t0);
SamplerState g_sampler_state : register(s0);

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer ConstantBuffer : register(b0)
{
    float4 _width_height;
    float4 _effect_param;
    float4 _tint;
    float4 _texture_param_0;
};

float CalculateAlpha(float in_x, float in_y, float in_radius)
{
    float distance = sqrt((in_x * in_x) + (in_y * in_y));
    float coverage = 1.0 - saturate(distance - in_radius);
    return coverage;
}

float CalculateCornerAlpha(
    float2 in_uv,
    float4 in_radius,
    float2 in_width_height
)
{
    float2 pixel_pos = in_uv * in_width_height;

    // data[bottom left, top left, top right, bottom right]

    float bottom_left = CalculateAlpha(
        max(0.0, in_radius.x - pixel_pos.x + 0.5),
        max(0.0, in_radius.x - (in_width_height.y - pixel_pos.y) + 0.5),
        in_radius.x
    );

    float top_left = CalculateAlpha(
        max(0.0, in_radius.y - pixel_pos.x + 0.5),
        max(0.0, in_radius.y - pixel_pos.y + 0.5),
        in_radius.y
    );

    float top_right = CalculateAlpha(
        max(0.0, in_radius.z - (in_width_height.x - pixel_pos.x) + 0.5),
        max(0.0, in_radius.z - pixel_pos.y + 0.5),
        in_radius.z
    );

    float bottom_right = CalculateAlpha(
        max(0.0, in_radius.w - (in_width_height.x - pixel_pos.x + 0.5)),
        max(0.0, in_radius.w - (in_width_height.y - pixel_pos.y + 0.5)),
        in_radius.w
    );

    return bottom_left * top_left * top_right * bottom_right;
}


Pixel main(Interpolant in_input)
{
    Pixel result;
    float2 uv = float2(
        in_input._uv.x * _texture_param_0.x / _texture_param_0.z,
        in_input._uv.y * _texture_param_0.y / _texture_param_0.w
        );
    float4 texel = g_texture.Sample(g_sampler_state, uv);

    float corner_alpha = CalculateCornerAlpha(
        in_input._uv,
        _effect_param,
        _width_height.xy
        );

    result._colour = texel * corner_alpha;

    return result;
}