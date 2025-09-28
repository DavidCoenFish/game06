#include "debug_grid_interpolant.hlsli"

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer ConstantBuffer : register(b0)
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
    colour += step(fmod(temp.x + temp.y, 2), 0) * 0.03125;
    colour += step(fmod((int)(temp.x * 0.25) + (int)(temp.y * 0.25), 2), 0) * 0.0625;
    colour += step(fmod((int)(temp.x * 0.0625) + (int)(temp.y * 0.0625), 2), 0) * 0.125;

    // grid lines on 64 pixel
    colour *= (1 - step(fmod(temp.x, 64), 0));
    colour *= (1 - step(fmod(temp.y, 64), 0));

    result._colour = float4(colour, colour, colour, 1.0);

    return result;
}