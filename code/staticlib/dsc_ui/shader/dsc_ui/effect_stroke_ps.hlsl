#include "effect_stroke_interpolant.hlsli"

Texture2D g_texture : register(t0);
SamplerState g_sampler_state : register(s0);

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer MyConstantBuffer : register(b0)
{
    float4 _width_height;
    float4 _effect_param; // radius
    float4 _tint;
    float4 _texture_param_0;
};

float SampleAtOffset(
    float2 in_pivot_pixel, 
    float2 in_offset_pixel, 
    float2 in_texture_size,
    float in_low,
    float in_high
    )
{
    float ratio = saturate((_effect_param.x - in_low) / (in_high - in_low));

    float sum = 0.0;
    sum += g_texture.Sample(g_sampler_state, (in_pivot_pixel + in_offset_pixel) / in_texture_size).a;
    sum += g_texture.Sample(g_sampler_state, (in_pivot_pixel - in_offset_pixel) / in_texture_size).a;
    float2 flip_offset = float2(in_offset_pixel.y, -in_offset_pixel.x);
    sum += g_texture.Sample(g_sampler_state, (in_pivot_pixel + flip_offset) / in_texture_size).a;
    sum += g_texture.Sample(g_sampler_state, (in_pivot_pixel - flip_offset) / in_texture_size).a;
    return sum * ratio;
}

float CalculateStrokeAlpha(
    float2 in_uv
)
{
    float2 pivot_pixel = floor(in_uv * _texture_param_0.xy) + float2(0.5, 0.5);
    float4 texel = g_texture.Sample(g_sampler_state, pivot_pixel / _texture_param_0.zw);

    float coverage = texel.a;

    coverage += SampleAtOffset(pivot_pixel, float2(0.5, 1.5), _texture_param_0.zw, 0.5, 2.9154);
    coverage += SampleAtOffset(pivot_pixel, float2(0.5, 3.5), _texture_param_0.zw, 2.5, 5.1478);
    coverage += SampleAtOffset(pivot_pixel, float2(0.5, 5.5), _texture_param_0.zw, 4.5, 6.6708);
    coverage += SampleAtOffset(pivot_pixel, float2(2.5, 1.5), _texture_param_0.zw, 1.5811, 4.3012);
    coverage += SampleAtOffset(pivot_pixel, float2(2.5, 3.5), _texture_param_0.zw, 2.9154, 5.7009);
    coverage += SampleAtOffset(pivot_pixel, float2(4.5, 1.5), _texture_param_0.zw, 3.5355, 6.0415);

    float result = saturate(coverage);
    return result;
}

Pixel main(Interpolant in_input)
{
    Pixel result;
    float2 uv = in_input._uv.xy * _texture_param_0.xy / _texture_param_0.zw;
    // the source pixel at the equivalent location we are about to return from this shader
    float4 texel = g_texture.Sample(g_sampler_state, uv);

    float stroke_alpha = CalculateStrokeAlpha(in_input._uv);
    float4 shadow_colour = _tint * stroke_alpha;

    // premultiplied blend
    //result = source.RGB + (dest.RGB * (1 - source.A))

    result._colour =
        float4(texel.rgb + (shadow_colour.rgb * (1.0 - texel.a)),
            texel.a + ((1.0 - texel.a) * shadow_colour.a)
            );

    //result._colour = float4(texel.a, texel.a, texel.a, 1.0);

    return result;
}