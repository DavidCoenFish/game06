#include "debug_grid_interpolant.hlsli"

Texture2D g_texture : register(t0);
SamplerState g_sampler_state : register(s0);

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer ConstantBuffer : register(b0)
{
    float4 _width_height;
    float4 _effect_param;
    float4 _tint;
    float4 _texture_param_0;
};

float2 SampleAtOffset(float2 in_coverage_shadow, float2 in_pivot_pixel, float2 in_offset_pixel, float2 in_texture_size)
{
    float sum = 0.0;
    sum += g_texture.Sample(g_sampler_state, (in_pivot_pixel + in_offset_pixel) / in_texture_size).a;
    sum += g_texture.Sample(g_sampler_state, (in_pivot_pixel - in_offset_pixel) / in_texture_size).a;
    float2 flip_offset = float2(in_offset_pixel.y, -in_offset_pixel.x);
    sum += g_texture.Sample(g_sampler_state, (in_pivot_pixel + flip_offset) / in_texture_size).a;
    sum += g_texture.Sample(g_sampler_state, (in_pivot_pixel - flip_offset) / in_texture_size).a;
    return float2(
        in_coverage_shadow.x + sum,
        in_coverage_shadow.y + 4.0
        );
}

float CalculateShadowAlpha(
    float2 in_uv,
    float in_shadow_strength
)
{
    //float2 pivot = (floor((in_uv * in_width_height) - in_offset)) + float2(0.5, 0.5);
    float2 pivot_pixel = floor((in_uv * _texture_param_0.xy) - _effect_param.xy) + float2(0.5, 0.5);

    float4 texel = g_texture.Sample(g_sampler_state, pivot_pixel / _texture_param_0.zw);

    float2 coverage_shadow = float2(texel.a, 1.0);

    coverage_shadow = SampleAtOffset(coverage_shadow, pivot_pixel, float2(0.5, 1.5), _texture_param_0.zw);
    coverage_shadow = SampleAtOffset(coverage_shadow, pivot_pixel, float2(0.5, 3.5), _texture_param_0.zw);
    coverage_shadow = SampleAtOffset(coverage_shadow, pivot_pixel, float2(0.5, 5.5), _texture_param_0.zw);
    coverage_shadow = SampleAtOffset(coverage_shadow, pivot_pixel, float2(2.5, 1.5), _texture_param_0.zw);
    coverage_shadow = SampleAtOffset(coverage_shadow, pivot_pixel, float2(2.5, 3.5), _texture_param_0.zw);
    coverage_shadow = SampleAtOffset(coverage_shadow, pivot_pixel, float2(4.5, 1.5), _texture_param_0.zw);

    float result = coverage_shadow.x / coverage_shadow.y;
    return result;
}

Pixel main(Interpolant in_input)
{
    Pixel result;
    //float2 uv = float2(
    //    in_input._uv.x * _texture_param_0.x / _texture_param_0.z,
    //    in_input._uv.y * _texture_param_0.y / _texture_param_0.w
    //    );
    float2 uv = in_input._uv.xy * _texture_param_0.xy / _texture_param_0.zw;
    // the source pixel at the equivalent location we are about to return from this shader
    float4 texel = g_texture.Sample(g_sampler_state, uv);

    float shadow_alpha = CalculateShadowAlpha(
        in_input._uv,
        _effect_param.z // shadow strength
        );
    float4 shadow_colour = _tint * shadow_alpha;

    // premultiplied blend
    //result = source.RGB + (dest.RGB * (1 - source.A))

    result._colour =
        float4(texel.rgb + (shadow_colour.rgb * (1.0 - texel.a)),
            texel.a + ((1.0 - texel.a) * shadow_colour.a)
            );

    return result;
}