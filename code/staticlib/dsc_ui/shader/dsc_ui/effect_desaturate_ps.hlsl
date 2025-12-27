#include "effect_desaturate_interpolant.hlsli"

Texture2D g_texture : register(t0);
SamplerState g_sampler_state : register(s0);

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer MyConstantBuffer : register(b0)
{
    float4 _width_height_strength;
    float4 _effect_param;
    float4 _tint;
    float4 _texture_param_0; // viewport size xy, texture size zw. the ui can draw to viewports/ subset areas of textures
};

Pixel main(Interpolant in_input)
{
    float2 uv = in_input._uv.xy * _texture_param_0.xy / _texture_param_0.zw;
    float4 texel = g_texture.Sample(g_sampler_state, uv);
    float luma = (0.3 * texel.r) + (0.6 * texel.g) + (0.1 * texel.b);
    float4 desaturate_colour = float4(luma, luma, luma, texel.a) * _tint;
    float4 result_colour = lerp(texel, desaturate_colour, _width_height_strength.z);

    Pixel result;
    result._colour = result_colour;

    return result;
}