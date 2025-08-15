#include "knot_interpolant.hlsli"

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

Pixel main(Interpolant in_input)
{
    Pixel result;

    result._colour = float4(1.0, 0.0, 0.0, 1.0);

    return result;
}