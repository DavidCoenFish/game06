#include "effect_burn_blot_interpolant.hlsli"

// the text or other linework, non alphaed areas should propergate burn/ blot faster
Texture2D g_texture_0 : register(t0);
SamplerState g_sampler_state_0 : register(s0);

// the previous state of the blot
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
    float4 _size; // our own pixel width, height, 
    float4 _param; // rolled over [0 ... 1] (1 == rolled over), time delta [0 ...], mouse zw in pixels, uv coords bottom left is 0,0
};

float GetAdjacentBurn(float2 in_uv, float2 in_pixel_step)
{
    float result = g_texture_0.Sample(g_sampler_state_0, in_uv + float2(in_pixel_step.x, 0.0)).x;
    result = max(result, g_texture_0.Sample(g_sampler_state_0, in_uv - float2(in_pixel_step.x, 0.0)).x);
    result = max(result, g_texture_0.Sample(g_sampler_state_0, in_uv + float2(0.0, in_pixel_step.y)).x);
    result = max(result, g_texture_0.Sample(g_sampler_state_0, in_uv - float2(0.0, in_pixel_step.y)).x);
    return result;
}

float GetFurtherMaxBurn(float2 in_uv, float2 in_pixel_step)
{
    float result = g_texture_0.Sample(g_sampler_state_0, in_uv + (float2(2.0, 0.0) * in_pixel_step)).x;
    result = max(result, g_texture_0.Sample(g_sampler_state_0, in_uv + (float2(1.0, 1.0) * in_pixel_step)).x);
    result = max(result, g_texture_0.Sample(g_sampler_state_0, in_uv + (float2(0.0, 2.0) * in_pixel_step)).x);
    result = max(result, g_texture_0.Sample(g_sampler_state_0, in_uv + (float2(-1.0, 1.0) * in_pixel_step)).x);
    result = max(result, g_texture_0.Sample(g_sampler_state_0, in_uv + (float2(-2.0, 0.0) * in_pixel_step)).x);
    result = max(result, g_texture_0.Sample(g_sampler_state_0, in_uv + (float2(-1.0, -1.0) * in_pixel_step)).x);
    result = max(result, g_texture_0.Sample(g_sampler_state_0, in_uv + (float2(0.0, -2.0) * in_pixel_step)).x);
    result = max(result, g_texture_0.Sample(g_sampler_state_0, in_uv + (float2(1.0, -1.0) * in_pixel_step)).x);
    return result;
}

Pixel main(Interpolant in_input)
{
    // the source pixel at the equivalent location we are about to return from this shader
    float2 pixel_step_0 = _texture_param_0.xy / _texture_param_0.zw;
    float2 uv_0 = in_input._uv.xy * pixel_step_0;
    float4 texel_0 = g_texture_0.Sample(g_sampler_state_0, uv_0);

    // the previous state of this pixel, accumulating burn in red
    float2 uv_1 = in_input._uv.xy * _texture_param_1.xy / _texture_param_1.zw;
    float4 texel_1 = g_texture_1.Sample(g_sampler_state_1, uv_1);
    float burn_accumulate = texel_1.x;
    float burn_time = texel_1.y;

    float rollover = _param.x;
    float time_delta = _param.y;
    float2 touch = _param.zw;

    // if we are burning, and in the rollover state, keep burning
    if (0.0 < burn_accumulate)
    {
        burn_accumulate += (rollover * time_delta * 4.0);
    }

    // if we are near the mouse cursor, start burning
    float2 pixel_uv_to_cursor = (in_input._uv.xy * _size.xy) - touch;
    float burn_seed = clamp((1.0 - dot(pixel_uv_to_cursor, pixel_uv_to_cursor)), 0.0, 1.0);
    burn_accumulate += (burn_seed * rollover * time_delta * 4.0);

    // if we are near burning pixels, start burning
    float adjacent_max_burn = GetAdjacentBurn(uv_0, pixel_step_0);
    if (1.0 <= adjacent_max_burn)
    {
        burn_accumulate += (rollover * time_delta * 4.0);
    }

    // if we are on non zero alpha of the source texture, add burn from slightly further
    float further_max_burn = GetFurtherMaxBurn(uv_0, pixel_step_0);
    if (1.0 <= adjacent_max_burn)
    {
        burn_accumulate += (rollover * time_delta * 4.0 * texel_0.a);
    }

    if (1.0 <= burn_accumulate)
    {
        burn_time += (rollover * time_delta);
    }

    // if we are not in rollover, degrade the burn accumulation
    burn_accumulate -= ((1.0 - rollover) * time_delta * 2.0f);
    burn_time *= rollover;

    Pixel result;
    result._colour = float4(
        clamp(burn_accumulate, 0.0, 1.0),
        clamp(burn_time, 0.0, 1.0),
        0.0, 1.0);

    return result;
}