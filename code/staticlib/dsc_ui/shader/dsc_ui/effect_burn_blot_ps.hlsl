#include "effect_burn_blot_interpolant.hlsli"

/*
reminder, this pixel shader is intented to be written out without blending, want it to just write out it rgba value without any blending

*/

// the text or other linework, non alphaed areas should propergate burn/ blot faster
Texture2D g_texture_0 : register(t0);
SamplerState g_sampler_state_0 : register(s0);

/*
red - burn accumulate
green - time accumulate on full red/ burn
blue - rollover accumulate
alpha - alpha from g_sampler_state_0, to help present only write burn onto relevant pixels
*/
// the previous state of the blot
Texture2D g_texture_1 : register(t1);
SamplerState g_sampler_state_1 : register(s1);

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer ConstantBuffer : register(b0)
{
    float4 _width_height;
    float4 _effect_param; // x. rolled over [0 ... 1] (1 == rolled over), y. time delta [0 ...], zw. mouse pos relative to shader in pixels, uv coords bottom left is 0,0
    float4 _tint;
    float4 _texture_param_0; // viewport size xy, texture size zw. the ui can draw to viewports/ subset areas of textures
    float4 _texture_param_1; // viewport size xy, texture size zw. the ui can draw to viewports/ subset areas of textures
};

float2 ClampUv(float2 in_uv, float2 in_pixel_step, float2 in_delta)
{
    return clamp(in_uv + (in_pixel_step * in_delta), in_pixel_step, 1.0 - in_pixel_step);
}

float GetAdjacentBurn(float2 in_uv, float2 in_pixel_step)
{
    float result = g_texture_1.Sample(g_sampler_state_1, ClampUv(in_uv, in_pixel_step, float2(1.0, 0.0))).x
        + g_texture_1.Sample(g_sampler_state_1, ClampUv(in_uv, in_pixel_step, float2(-1.0, 0.0))).x
        + g_texture_1.Sample(g_sampler_state_1, ClampUv(in_uv, in_pixel_step, float2(0.0, 1.0))).x
        + g_texture_1.Sample(g_sampler_state_1, ClampUv(in_uv, in_pixel_step, float2(0.0, -1.0))).x;
    return result;
}

float GetFurtherMaxBurn(float2 in_uv, float2 in_pixel_step)
{
    float result = g_texture_1.Sample(g_sampler_state_1, ClampUv(in_uv, in_pixel_step, float2(2.0, 0.0))).x
        + g_texture_1.Sample(g_sampler_state_1, ClampUv(in_uv, in_pixel_step, float2(1.0, 1.0))).x
        + g_texture_1.Sample(g_sampler_state_1, ClampUv(in_uv, in_pixel_step, float2(0.0, 2.0))).x
        + g_texture_1.Sample(g_sampler_state_1, ClampUv(in_uv, in_pixel_step, float2(-1.0, 1.0))).x
        + g_texture_1.Sample(g_sampler_state_1, ClampUv(in_uv, in_pixel_step, float2(-2.0, 0.0))).x
        + g_texture_1.Sample(g_sampler_state_1, ClampUv(in_uv, in_pixel_step, float2(-1.0, -1.0))).x
        + g_texture_1.Sample(g_sampler_state_1, ClampUv(in_uv, in_pixel_step, float2(0.0, -2.0))).x
        + g_texture_1.Sample(g_sampler_state_1, ClampUv(in_uv, in_pixel_step, float2(1.0, -1.0))).x;
    return result;
}

Pixel main(Interpolant in_input)
{
    Pixel result;

    // the source pixel at the equivalent location we are about to return from this shader
    //float2 uv_scale_0 = _texture_param_0.xy / _texture_param_0.zw;
    //float2 pixel_step_0 = 1.0 / _texture_param_0.zw;
    float2 uv_0 = in_input._uv.xy * _texture_param_0.xy / _texture_param_0.zw;
    float4 source_texel = g_texture_0.Sample(g_sampler_state_0, uv_0);

    float rollover = _effect_param.x;
    float time_delta = _effect_param.y;
    float2 touch = _effect_param.zw;

    // the previous state of this pixel, accumulating burn in red
    float2 pixel_step_1 = 1.0 / _texture_param_1.zw;
    float2 uv_1 = in_input._uv.xy * _texture_param_1.xy / _texture_param_1.zw;
    float4 blot_texel = g_texture_1.Sample(g_sampler_state_1, uv_1);
    float burn_accumulate = blot_texel.r;
    float burn_time = blot_texel.g;
    float rollover_accumulate = blot_texel.b;

    //0.00390625 = 1 / 256
    if (0.5 < rollover)
    {
        rollover_accumulate += clamp(2.0 * time_delta, 0.004, 1.0f);
    }
    else
    {
        rollover_accumulate -= clamp(3.0 * time_delta, 0.004, 1.0f);
    }
    rollover_accumulate = clamp(rollover_accumulate, 0.0, 1.0);

    // reduce the burn accumulate if we are not a full intensity, but only a bit, the fade out is pretty abbrupt
    if (rollover_accumulate < 0.25)
    {
        burn_accumulate *= (rollover_accumulate + 0.25);
    }

    // if we are burning, and in the rollover state, keep burning
    if (0.0 < burn_accumulate)
    {
        burn_accumulate += (rollover * time_delta * 4.0);
    }

    // if we are near the mouse cursor, start burning
    float2 pixel_uv_to_cursor = (in_input._uv.xy * _width_height.xy) - touch;
    float burn_seed = clamp((5.0 - dot(pixel_uv_to_cursor, pixel_uv_to_cursor)), 0.0, 1.0);
    //float burn_seed = clamp((1.0 - dot(pixel_uv_to_cursor, pixel_uv_to_cursor)), 0.0, 1.0);
    //float burn_seed = clamp((10.0 - dot(pixel_uv_to_cursor, pixel_uv_to_cursor)), 0.0, 1.0);
    burn_accumulate += (burn_seed * rollover * time_delta * 4.0);

    // if we are near burning pixels, start burning
    float adjacent_max_burn = GetAdjacentBurn(uv_1, pixel_step_1);
    if (1.0 <= adjacent_max_burn)
    {
        burn_accumulate += (rollover * time_delta * 16.0 * adjacent_max_burn);
    }

    // if we are on non zero alpha of the source texture, add burn from slightly further
    float further_max_burn = GetFurtherMaxBurn(uv_1, pixel_step_1);
    if (1.0 <= further_max_burn)
    {
        burn_accumulate += (rollover * time_delta * 32.0 * further_max_burn * source_texel.a);
    }

    if (1.0 <= burn_accumulate)
    {
        burn_time += (rollover * time_delta * 0.5);
    }

    // if we are not in rollover, degrade the burn accumulation
    //burn_accumulate -= ((1.0 - rollover) * time_delta * 2.0f);
    if (rollover_accumulate < 0.25)
    {
        burn_time *= (rollover_accumulate + 0.25);
    }

    result._colour = float4(
        clamp(burn_accumulate, 0.0, 1.0),
        clamp(burn_time, 0.0, 1.0),
        rollover_accumulate, 
        1.0);

    return result;
}