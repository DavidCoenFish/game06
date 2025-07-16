#include "effect_burn_present_interpolant.hlsli"

// the text or other linework
Texture2D g_texture_0 : register(t0);
SamplerState g_sampler_state_0 : register(s0);

// the burn blot state, accumulating burn in red, time burning in green, time since rollover in blue?
Texture2D g_texture_1 : register(t1);
SamplerState g_sampler_state_1 : register(s1);

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer ConstantBuffer : register(b0)
{
    float4 _width_height;
    float4 _effect_param;
    float4 _tint;
    float4 _texture_param_0; // viewport size xy, texture size zw. the ui can draw to viewports/ subset areas of textures
    float4 _texture_param_1; // viewport size xy, texture size zw. the ui can draw to viewports/ subset areas of textures
};

Pixel main(Interpolant in_input)
{
    float2 uv_0 = in_input._uv.xy * _texture_param_0.xy / _texture_param_0.zw;
    float4 source_texel = g_texture_0.Sample(g_sampler_state_0, uv_0);

    float2 uv_1 = in_input._uv.xy * _texture_param_1.xy / _texture_param_1.zw;
    float4 blot_texel = g_texture_1.Sample(g_sampler_state_1, uv_1);
    float burn_amount = blot_texel.r;
    float burn_time = blot_texel.g;
    float burn_alpha = blot_texel.b;

    Pixel result;
    result._colour = source_texel;

    if (1.0 <= burn_amount)
    {
        float temp1 = clamp((1.0 - (13.0 * burn_time)), 0.0, 1.1);
        float temp2 = clamp((1.0 - (11.0 * burn_time)), 0.0, 1.05);
        float temp3 = clamp((1.1 - (6.5 * burn_time)), 0.0, 1.0);

        float4 burn_colour = float4(
            temp3,
            temp2,
            temp1,
            burn_alpha //* source_texel.a
            );

        //blend colour in place
        float alpha = burn_alpha * source_texel.a;
        result._colour.rgb = (source_texel.rgb * (1.0 - alpha)) + (burn_colour.rgb * alpha);
    }

    return result;
}