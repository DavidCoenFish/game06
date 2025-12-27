#include "present_pos.hlsli"

cbuffer ConstantBuffer1 : register(b1)
{
    float4 _colour;
};

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

Pixel main(Interpolant in_input)
{
    Pixel result;

    //result._colour = float4(1.0f, 1.0f, 1.0f, 1.0f);
    result._colour = _colour * abs(1.0 - in_input._position.z);
    //result._colour.r = abs(1.0 - in_input._position.z);

    return result;
}