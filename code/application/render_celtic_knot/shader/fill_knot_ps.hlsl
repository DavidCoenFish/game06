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
    float4 _data_size;
};

Pixel main(Interpolant in_input)
{
    Pixel result;

    float2 pixel_uv = in_input._uv * _texture_size_knot_size.xy;
    float2 knot_size = float2(_texture_size_knot_size.z, _texture_size_knot_size.z);
    float2 knot_ui = fmod(pixel_uv, knot_size) / knot_size;

    float2 data_index = floor(pixel_uv / knot_size);
    float a = g_texture_data.Sample(g_sampler_state_data, (data_index + float2(0.0, 0.0)) / _data_size.xy).r;
    float b = g_texture_data.Sample(g_sampler_state_data, (data_index + float2(1.0, 0.0)) / _data_size.xy).r;
    float c = g_texture_data.Sample(g_sampler_state_data, (data_index + float2(0.0, 1.0)) / _data_size.xy).r;
    float d = g_texture_data.Sample(g_sampler_state_data, (data_index + float2(1.0, 1.0)) / _data_size.xy).r;
    float s = a + b + c + d;

    float4 mask = float4(0.0, 0.0, 0.0, 0.0);
    if (s == 1)
    {
        mask = float4(0.0, 0.0, 1.0, 0.0);

        if (b == 1)
        {
            knot_ui = float2(knot_ui.y, -knot_ui.x);
        }
        else if (c == 1)
        {
            knot_ui = float2(-knot_ui.y, knot_ui.x);
        }
        else if (d == 1)
        {
            knot_ui = float2(-knot_ui.x, -knot_ui.y);
        }
    }
    else if ((s == 2) && ((a != d) || (b != c)))
    {
        mask = float4(0.0, 1.0, 0.0, 0.0);

        if ((b == 1) && (d == 1))
        {
            knot_ui = float2(knot_ui.y, -knot_ui.x);
        }
        else if ((a == 1) && (c == 1))
        {
            knot_ui = float2(-knot_ui.y, knot_ui.x);
        }
        else if ((c == 1) && (d == 1))
        {
            knot_ui = float2(-knot_ui.x, -knot_ui.y);
        }
    }
    else if (2.0 <= s)
    {
        mask = float4(1.0, 0.0, 0.0, 0.0);
    }

    float4 texel_knot = g_texture_knot.Sample(g_sampler_state_knot, knot_ui) * mask;
    float value = texel_knot.x + texel_knot.y + texel_knot.z + texel_knot.w;

    result._colour = float4(
        value,
        value,
        value,
        1.0);

    return result;
}