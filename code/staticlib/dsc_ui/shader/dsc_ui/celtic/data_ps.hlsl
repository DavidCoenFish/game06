#include "data_interpolant.hlsli"

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

cbuffer MyConstantBuffer : register(b0)
{
    // data size [x,y] for the celtic knot data map, then [z,w] of the render target size
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

    float2 pixel_uv = in_input._uv * _texture_size.zw;
    float2 middle_data = _texture_size.xy * 0.5;
    //mirror
    pixel_uv = floor(min(pixel_uv, middle_data - (pixel_uv - middle_data)));
    float df = saturate(length((pixel_uv - middle_data) / middle_data));
    float min_dim = min(_texture_size.x, _texture_size.y);
    float hollow_factor = saturate((min_dim - 3.0) / 5.0);
    float mul = 1.0 - (hollow_factor * (1.0 - (df * df)));

    float value = random(pixel_uv) * mul;
    value += 0.35 * saturate(4 - (min_dim * 0.5));
    if ((pixel_uv.x == 2.0) ||
        (pixel_uv.y == 2.0))
    {
        value += 0.35;
    }
    if ((pixel_uv.x <= 0) ||
        (pixel_uv.y <= 0))
    {
        value = 0;
    }

    value = 1 - step(value, 0.5);
    result._colour = float4(value, value, value, 1.0);
    //result._colour = float4(mul, mul, mul, 1.0);

    return result;
}