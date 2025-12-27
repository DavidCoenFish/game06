#include "present_interpolant.hlsli"

Texture2D g_texture : register(t0);
SamplerState g_sampler_state : register(s0);

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

Pixel main(Interpolant in_input)
{
    Pixel result;

    float4 texel = g_texture.Sample(g_sampler_state, in_input._uv);

    result._colour = texel;

    return result;
}