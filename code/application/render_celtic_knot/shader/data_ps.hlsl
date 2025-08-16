#include "data_interpolant.hlsli"

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer ConstantBuffer : register(b0)
{
    // we want to know the texture size so we can smooth pixel edges
    float4 _texture_size;
};

// https://stackoverflow.com/questions/5149544/can-i-generate-a-random-number-inside-a-pixel-shader
float random(float2 p)
{
    float2 K1 = float2(
        23.14069263277926, // e^pi (Gelfond's constant)
        2.665144142690225 // 2^sqrt(2) (Gelfond–Schneider constant)
    );
    return frac(cos(dot(p, K1)) * 12345.6789);
}

Pixel main(Interpolant in_input)
{
    Pixel result;

    float value = step(random(in_input._uv), 0.33);

    result._colour = float4(
        value,
        value,
        value,
        1.0);

    return result;
}