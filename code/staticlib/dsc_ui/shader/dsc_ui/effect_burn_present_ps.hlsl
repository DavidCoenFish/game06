#include "effect_burn_present_interpolant.hlsli"

// the text or other linework
Texture2D g_texture_0 : register(t0);
SamplerState g_sampler_state_0 : register(s0);

// the burn blot state
Texture2D g_texture_1 : register(t1);
SamplerState g_sampler_state_1 : register(s1);


struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer ConstantBuffer : register(b0)
{
    float4 _texture_param_0; // viewport size xy, texture size zw. the ui can draw to viewports/ subset areas of textures
    float4 _texture_param_1; // viewport size xy, texture size zw. the ui can draw to viewports/ subset areas of textures
    //float4 _size; // output pixel size, width, height
};

Pixel main(Interpolant in_input)
{
    // the source pixel at the equivalent location we are about to return from this shader
    float2 uv_0 = in_input._uv.xy * _texture_param_0.xy / _texture_param_0.zw;
    float4 texel_0 = g_texture_0.Sample(g_sampler_state_0, uv_0);

    // accumulating burn in red, time burning in green, time since rollover in blue?
    float2 uv_1 = in_input._uv.xy * _texture_param_1.xy / _texture_param_1.zw;
    float4 burn = g_texture_1.Sample(g_sampler_state_1, uv_1);



    result._colour =
        float4(texel.rgb + (shadow_colour.rgb * (1.0 - texel.a)),
            texel.a + ((1.0 - texel.a) * shadow_colour.a)
            );

    return result;
}