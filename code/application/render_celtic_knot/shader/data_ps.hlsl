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



    float2 pixel_uv = float2(in_input._uv.x, 1.0 - in_input._uv.y) * _texture_size.zw;
    float2 middle_data = _texture_size.xy * 0.5;
    //mirror
    pixel_uv = floor(min(pixel_uv, middle_data - (pixel_uv - middle_data)));
    //float df = saturate(length((pixel_uv - middle_data) / middle_data));
    //float2 index_uv = floor(pixel_uv); // in_input._uv* _texture_size.zw);
    //df = df * df * df * df;
    //float value = step(1.0 - (random(pixel_uv) * df), 0.1);
    float value = random(pixel_uv);
    //value = df * df;
    value = step(value, 0.33);

    if ((pixel_uv.x <= 0) ||
        (pixel_uv.y <= 0))
    {
        value = 0.0;
    }


    result._colour = float4(
        value,
        value,
        value,
        1.0);

    return result;
}