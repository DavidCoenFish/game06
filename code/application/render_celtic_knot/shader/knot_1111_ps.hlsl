#include "knot_interpolant.hlsli"

struct Pixel
{
    float4 _colour : SV_TARGET0;
};

float CalculateRay(float in_value, float2 in_pos, float2 in_ray_origin, float2 in_ray_dir, float in_ray_thickness)
{
    float2 pos = in_pos - in_ray_origin;
    float t = dot(in_ray_dir, pos);
    float d = abs(dot(float2(in_ray_dir.y, -(in_ray_dir.x)), pos));
    float value_x = max(0.0, (in_ray_thickness - d) * 512.0);
    float value_y = max(0.0, t * 512.0);
    float result = max(in_value, min(value_x, value_y));
    return result;
}

//0.70710678118654752440084436210485
//1.4142135623730950488016887242097
float Calculate(float2 in_pos)
{
    float result = 0.0;
    result = CalculateRay(
        result,
        in_pos,
        float2(0.625, 0.125),
        float2(0.70710678118654, 0.70710678118654),
        0.08838834764831844055010554526311
        );

    result = CalculateRay(
        result,
        in_pos,
        float2(0.875, 0.625),
        float2(-0.70710678118654, 0.70710678118654),
        0.08838834764831844055010554526311
    );

    result = CalculateRay(
        result,
        in_pos,
        float2(0.375, 0.875),
        float2(-0.70710678118654, -0.70710678118654),
        0.08838834764831844055010554526311
    );

    result = CalculateRay(
        result,
        in_pos,
        float2(0.125, 0.375),
        float2(0.70710678118654, -0.70710678118654),
        0.08838834764831844055010554526311
    );

    return saturate(result);
}

Pixel main(Interpolant in_input)
{
    Pixel result;

    float value = Calculate(in_input._uv);

    result._colour = float4(value, value, value, 1.0);

    return result;
}