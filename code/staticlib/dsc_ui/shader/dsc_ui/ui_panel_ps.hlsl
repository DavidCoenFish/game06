#include "ui_panel_interpolant.hlsli"

Texture2D g_texture : register(t0);
SamplerState g_sampler_state : register(s0);

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer ConstantBufferEffect : register(b0)
{
    float4 _scroll_x_y;
    //float4 _width_height;
    //float4 _mouse_pos_time_rollover;
};

Pixel main(Interpolant in_input)
{
    Pixel result;
    float2 uv = in_input._uv_0 + ((in_input._uv_1 - in_input._uv_0) * _scroll_x_y);
    float4 texel = g_texture.Sample(g_sampler_state, uv);

    result._colour = texel;

    return result;
}