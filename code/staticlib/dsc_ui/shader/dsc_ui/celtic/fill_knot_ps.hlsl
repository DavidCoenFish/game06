#include "fill_knot_interpolant.hlsli"

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

// the data map of cell solid or blank
Texture2D g_texture_data : register(t0);
SamplerState g_sampler_state_data : register(s0);

// the celtic knot component texture, red 1111, green 1100, blue 1000
Texture2D g_texture_knot : register(t1);
SamplerState g_sampler_state_knot : register(s1);

cbuffer ConstantBuffer : register(b0)
{
    // width, height of our render target, then the size of the knot
    float4 _texture_size_knot_size;
	// width, height of the data texture, then a pixel delta x, delta y to get the knot centered on the render target
    float4 _data_size;
    float4 _knot_tint;
};

Pixel main(Interpolant in_input)
{
    Pixel result;

    float2 uv = in_input._uv + (_data_size.zw / _texture_size_knot_size.xy);

    float2 pixel_uv = uv * _texture_size_knot_size.xy;
    float2 knot_size = float2(_texture_size_knot_size.z, _texture_size_knot_size.z);
    float2 knot_uv = fmod(pixel_uv, knot_size) / knot_size;

    float2 data_index = floor(pixel_uv / knot_size);
    float a = g_texture_data.Sample(g_sampler_state_data, (data_index + float2(0.5, 0.5)) / _data_size.xy).r;
    float b = g_texture_data.Sample(g_sampler_state_data, (data_index + float2(1.5, 0.5)) / _data_size.xy).r;
    float c = g_texture_data.Sample(g_sampler_state_data, (data_index + float2(0.5, 1.5)) / _data_size.xy).r;
    float d = g_texture_data.Sample(g_sampler_state_data, (data_index + float2(1.5, 1.5)) / _data_size.xy).r;
    float s = a + b + c + d;

    float4 mask = float4(0.0, 0.0, 0.0, 0.0);
    if (s == 1)
    {
        mask = float4(0.0, 0.0, 1.0, 0.0);

        if (b == 1)
        {
            knot_uv = float2(knot_uv.y, 1.0 - knot_uv.x);
        }
        else if (c == 1)
        {
            knot_uv = float2(1.0 - knot_uv.y, knot_uv.x);
        }
        else if (d == 1)
        {
            knot_uv = float2(1.0 - knot_uv.x, 1.0 - knot_uv.y);
        }
    }
    else if ((s == 2) && ((a != d) || (b != c)))
    {
        mask = float4(0.0, 1.0, 0.0, 0.0);

        if ((b == 1) && (d == 1))
        {
            knot_uv = float2(knot_uv.y, 1.0 - knot_uv.x);
        }
        else if ((a == 1) && (c == 1))
        {
            knot_uv = float2(1.0 - knot_uv.y, knot_uv.x);
        }
        else if ((c == 1) && (d == 1))
        {
            knot_uv = float2(1.0 - knot_uv.x, 1.0 - knot_uv.y);
        }
    }
    else if (2.0 <= s)
    {
        mask = float4(1.0, 0.0, 0.0, 0.0);
    }

    knot_uv.y = 1.0 - (knot_uv.y);
    knot_uv *= (_texture_size_knot_size.z / _texture_size_knot_size.w);

    float4 texel_knot = g_texture_knot.Sample(g_sampler_state_knot, knot_uv) * mask;
    float value = texel_knot.x + texel_knot.y + texel_knot.z + texel_knot.w;

    result._colour = _knot_tint * value;

    //result._colour.r = knot_uv.x;
    //result._colour.g = knot_uv.y;
    //result._colour = float4(a, a, a, 1.0);

    return result;
}