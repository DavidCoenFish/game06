#include "effect_inner_shadow_interpolant.hlsli"

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

float2 SampleAtOffset(
    float2 in_coverage_shadow, 
    float2 in_pivot_pixel, 
    float2 in_offset_pixel, 
    float2 in_texture_size,
    float in_radius,
    float in_low,
    float in_high
    )
{
    float ratio = saturate((in_radius - in_low) / (in_high - in_low));

    float sum = 0.0;
    sum += (1.0 - g_texture.Sample(g_sampler_state, (in_pivot_pixel + in_offset_pixel) / in_texture_size).a);
    sum += (1.0 - g_texture.Sample(g_sampler_state, (in_pivot_pixel - in_offset_pixel) / in_texture_size).a);
    float2 flip_offset = float2(in_offset_pixel.y, -in_offset_pixel.x);
    sum += (1.0 - g_texture.Sample(g_sampler_state, (in_pivot_pixel + flip_offset) / in_texture_size).a);
    sum += (1.0 - g_texture.Sample(g_sampler_state, (in_pivot_pixel - flip_offset) / in_texture_size).a);
    return float2(
        in_coverage_shadow.x + (sum * ratio),
        in_coverage_shadow.y + (4.0 * ratio)
        );
}

float CalculateShadowAlpha(
    float2 in_uv
)
{
    //float2 pivot = (floor((in_uv * in_width_height) - in_offset)) + float2(0.5, 0.5);
    float2 pivot_pixel = floor((in_uv * _texture_param_0.xy) - _effect_param.xy) + float2(0.5, 0.5);

    float4 texel = g_texture.Sample(g_sampler_state, pivot_pixel / _texture_param_0.zw);
    float radius = _effect_param.z;
    float2 coverage_shadow = float2(1.0 - texel.a, 1.0);

    coverage_shadow = SampleAtOffset(coverage_shadow, pivot_pixel, float2(0.5, 1.5), _texture_param_0.zw, radius, 0.5, 2.9154);
    coverage_shadow = SampleAtOffset(coverage_shadow, pivot_pixel, float2(0.5, 3.5), _texture_param_0.zw, radius, 2.5, 5.1478);
    coverage_shadow = SampleAtOffset(coverage_shadow, pivot_pixel, float2(0.5, 5.5), _texture_param_0.zw, radius, 4.5, 6.6708);
    coverage_shadow = SampleAtOffset(coverage_shadow, pivot_pixel, float2(2.5, 1.5), _texture_param_0.zw, radius, 1.5811, 4.3012);
    coverage_shadow = SampleAtOffset(coverage_shadow, pivot_pixel, float2(2.5, 3.5), _texture_param_0.zw, radius, 2.9154, 5.7009);
    coverage_shadow = SampleAtOffset(coverage_shadow, pivot_pixel, float2(4.5, 1.5), _texture_param_0.zw, radius, 3.5355, 6.0415);

    float result = coverage_shadow.x / coverage_shadow.y;
    return result;
}

Pixel main(Interpolant in_input)
{
    Pixel result;
    float2 uv = in_input._uv.xy * _texture_param_0.xy / _texture_param_0.zw;
    // the source pixel at the equivalent location we are about to return from this shader
    float4 texel = g_texture.Sample(g_sampler_state, uv);

    float shadow_alpha = CalculateShadowAlpha(
        in_input._uv
        );
    float4 shadow_colour = _tint * shadow_alpha;

    // premultiplied blend
    //result = source.RGB + (dest.RGB * (1 - source.A))

    // we are replacing the existing pixel, but blending with the shad as appropriate
    result._colour =
        float4((texel.rgb * (1.0 - shadow_colour.a)) + (shadow_colour.rgb * shadow_colour.a),
            texel.a
            );

    return result;
}