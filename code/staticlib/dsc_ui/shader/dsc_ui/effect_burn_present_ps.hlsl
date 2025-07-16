#include "effect_burn_present_interpolant.hlsli"

// the text or other linework, non alphaed areas should propergate burn/ blot faster
Texture2D g_texture_0 : register(t0);
SamplerState g_sampler_state_0 : register(s0);


// the burn blot state
Texture2D g_texture_1 : register(t1);
SamplerState g_sampler_state_1 : register(s1);

/*
pixel output is intended to be premultiply alpha type blended with initial source linework image
*/

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

    // accumulating burn in red, time burning in green, time since rollover in blue?

    float2 uv_1 = in_input._uv.xy * _texture_param_1.xy / _texture_param_1.zw;
    float4 blot_texel = g_texture_1.Sample(g_sampler_state_1, uv_1);
    float burn_amount = blot_texel.r;
    float burn_time = blot_texel.g;
    float burn_alpha = blot_texel.b;

    Pixel result;
    result._colour = source_texel;

    if (1.0 <= burn_amount)
    {
        //float temp1 = clamp((1.0 - (20.0 * burn_time)), 0.0, 1.0);
        //float temp2 = clamp((1.0 - (17.0 * burn_time)), 0.0, 1.0);
        //float temp3 = clamp((1.1 - (10.0 * burn_time)), 0.0, 1.0);

        float temp1 = clamp((1.0 - (13.0 * burn_time)), 0.0, 1.0);
        float temp2 = clamp((1.0 - (11.0 * burn_time)), 0.0, 1.0);
        float temp3 = clamp((1.1 - (6.5 * burn_time)), 0.0, 1.0);

        float4 burn_colour = float4(
            temp3,
            temp2,
            temp1,
            burn_alpha //* source_texel.a
            );

        //blend alpha
        //https://microsoft.github.io/Win2D/WinUI3/html/PremultipliedAlpha.htm
        //result = source.RGB + (dest.RGB * (1 - source.A))
        //result._colour += (burn_colour * (1.0 - source_texel.a));
        //result._colour += burn_colour;
        result._colour.rgb = (source_texel.rgb * (1.0 - burn_alpha)) + (burn_colour.rgb * burn_alpha * source_texel.a);
        //result._colour += (burn_colour * 0.04);
    }
    //result._colour = source_texel;

    return result;
}