#include "effect_round_corner_interpolant.hlsli"

Texture2D g_texture : register(t0);
SamplerState g_sampler_state : register(s0);

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer MyConstantBuffer : register(b0)
{
    float4 _width_height;
    float4 _effect_param;
    float4 _tint;
    float4 _texture_param_0;
    float4 _texture_param_1;
};

Pixel main(Interpolant in_input)
{
    Pixel result;
    float2 uv = float2(
        in_input._uv.x * _texture_param_0.x / _texture_param_0.z,
        in_input._uv.y * _texture_param_0.y / _texture_param_0.w
        );
    float4 texel = g_texture.Sample(g_sampler_state, uv);

    // premultiplied blend
    //result = source.RGB + (dest.RGB * (1 - source.A))

    result._colour = 
        float4(_tint.rgb + (texel.rgb * (1.0 - _tint.a)),
            _tint.a + ((1.0 - _tint.a) * texel.a)
            );

    return result;
}