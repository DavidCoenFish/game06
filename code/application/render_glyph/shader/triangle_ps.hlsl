#include "triangle_interpolant.hlsli"

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

Pixel main(Interpolant in_input)
{
    Pixel result;

    // Premultiplied alpha blending
    result._colour = float4(
        in_input._colour.x,
        in_input._colour.y,
        in_input._colour.z,
        in_input._colour.w
        );

    return result;
}