#include "fill_interpolant.hlsli"

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer MyConstantBuffer : register(b0)
{
    float4 _colour;
};

Pixel main(Interpolant in_input)
{
    Pixel result;

    result._colour = _colour;

    return result;
}