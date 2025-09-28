#include "ui_panel_interpolant.hlsli"


Texture2D g_texture : register(t0);
SamplerState g_sampler_state : register(s0);

cbuffer ConstantBuffer1 : register(b1)
{
    float4 _tint;
};

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

Pixel main(Interpolant in_input)
{
    Pixel result;
    float4 texel = g_texture.Sample(g_sampler_state, in_input._uv);

    result._colour = texel * _tint;
    //result._colour = float4(1.0, 0.0, 0.0, 1.0);
    //result._colour = _tint;

    return result;
}