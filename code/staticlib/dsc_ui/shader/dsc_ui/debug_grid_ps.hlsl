#include "debug_grid_interpolant.hlsli"

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer ConstantBufferEffect : register(b0)
{
    float4 _width_height;
};

Pixel main(Interpolant in_input)
{
    Pixel result;

    int2 temp = int2(
        in_input._uv.x * _width_height.x,
        in_input._uv.y * _width_height.y
        );

    float colour = 0.25;
    //colour += step(((int)round(temp.x + temp.y)) % 2, 0) * 0.125;
    colour += step(fmod(((int)round((temp.x) + (temp.y))), 2), 0) * (0.5 * 0.7);
    
    colour += step(fmod(((int)round((temp.x * 0.25) + (temp.y * 0.25))), 2), 0) * (0.5 * 0.5);
    colour += step(fmod(((int)round((temp.x * 0.0625) + (temp.y * 0.0625))), 2), 0) * (0.5 * 0.3);
    colour += step(fmod(((int)round((temp.x * 0.015625) + (temp.y * 0.015625))), 2), 0) * (0.5 * 0.1);

    // grid lines on 64 pixel
    colour *= (1 - step(fmod(temp.x, 64), 0));
    colour *= (1 - step(fmod(temp.y, 64), 0));

    result._colour = float4(colour, colour, colour, 1.0);

    return result;
}