#include "effect_blur_interpolant.hlsli"

Texture2D g_texture : register(t0);
SamplerState g_sampler_state : register(s0);

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer ConstantBuffer : register(b0)
{
    float4 _width_height_strength;
    float4 _effect_param;
    float4 _tint;
    float4 _texture_param_0; // viewport size xy, texture size zw. the ui can draw to viewports/ subset areas of textures
};

float4 SampleAtOffset(
    float2 in_pivot_pixel, 
    float2 in_offset_pixel, 
    float2 in_texture_size
    )
{
    float4 sum = g_texture.Sample(g_sampler_state, (in_pivot_pixel + in_offset_pixel) / in_texture_size);
    sum += g_texture.Sample(g_sampler_state, (in_pivot_pixel - in_offset_pixel) / in_texture_size);
    float2 flip_offset = float2(in_offset_pixel.y, -in_offset_pixel.x);
    sum += g_texture.Sample(g_sampler_state, (in_pivot_pixel + flip_offset) / in_texture_size);
    sum += g_texture.Sample(g_sampler_state, (in_pivot_pixel - flip_offset) / in_texture_size);
    sum *= 0.04; // 1 / 25
    return sum;
}

float4 CalculateBlur(
    float2 in_uv
)
{
    float2 pivot_pixel = floor(in_uv * _texture_param_0.xy) + float2(0.5, 0.5);
    float4 texel = g_texture.Sample(g_sampler_state, pivot_pixel / _texture_param_0.zw);
    float4 average_colour = texel * 0.04; // 1 / 25

    average_colour += SampleAtOffset(pivot_pixel, float2(0.5, 1.5), _texture_param_0.zw);
    average_colour += SampleAtOffset(pivot_pixel, float2(0.5, 3.5), _texture_param_0.zw);
    average_colour += SampleAtOffset(pivot_pixel, float2(0.5, 5.5), _texture_param_0.zw);
    average_colour += SampleAtOffset(pivot_pixel, float2(2.5, 1.5), _texture_param_0.zw);
    average_colour += SampleAtOffset(pivot_pixel, float2(2.5, 3.5), _texture_param_0.zw);
    average_colour += SampleAtOffset(pivot_pixel, float2(4.5, 1.5), _texture_param_0.zw);

    float4 result_colour = lerp(texel, average_colour, _width_height_strength.z);

    return result_colour;
}

Pixel main(Interpolant in_input)
{
    float2 uv = in_input._uv.xy * _texture_param_0.xy / _texture_param_0.zw;
    // the source pixel at the equivalent location we are about to return from this shader
    float4 texel = g_texture.Sample(g_sampler_state, uv);

    Pixel result;
    result._colour = CalculateBlur(
        in_input._uv
        );

    return result;
}