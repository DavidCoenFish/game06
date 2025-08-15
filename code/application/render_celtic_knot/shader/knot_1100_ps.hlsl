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
    float value_y = max(0.0, (t * 512.0) + 1.0);
    float result = max(in_value, min(value_x, value_y));
    return result;
}

float CalculateRaySegment(float in_value, float2 in_pos, float2 in_ray_origin, float2 in_ray_dir, float in_ray_thickness, float in_length)
{
    float2 pos = in_pos - in_ray_origin;
    float t = dot(in_ray_dir, pos);
    float d = abs(dot(float2(in_ray_dir.y, -(in_ray_dir.x)), pos));
    float value_x = max(0.0, (in_ray_thickness - d) * 512.0);
    float value_t0 = max(0.0, (t * 512.0) + 1.0);
    float value_t1 = max(0.0, ((in_length - t) * 512.0) + 1.0);
    float result = max(in_value, min(value_x, min(value_t0, value_t1)));
    return result;
}

float CalculateArc(float in_value, float2 in_pos, float in_radius, float in_thickness, float2 in_clip_dir_0, float2 in_clip_dir_1)
{
    float2 pos = in_pos - float2(0.5, 0.5);
    float d = length(pos);

    float value_d = max(0.0, (in_thickness - abs(in_radius - d)) * 512.0);

    float c0 = dot(in_clip_dir_0, pos);
    float value_c0 = max(0.0, (c0 * 512.0) + 1.0);

    float c1 = dot(in_clip_dir_1, pos);
    float value_c1 = max(0.0, (c1 * 512.0) + 1.0);

    float result = max(in_value, min(value_d, min(value_c0, value_c1)));
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
        float2(0.375, 0.875),
        float2(-0.70710678118654, -0.70710678118654),
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
        float2(0.75, 0.25),
        float2(0.70710678118654, 0.70710678118654),
        0.08838834764831844055010554526311
    );

    result = CalculateArc(
        result,
        in_pos,
        0.35355339059327376220042218105242,
        0.08838834764831844055010554526311,
        float2(-0.70710678118654, -0.70710678118654),
        float2(0.70710678118654, -0.70710678118654)
    );

    result = CalculateRaySegment(
        result,
        in_pos,
        float2(0.125, 0.375),
        float2(0.70710678118654, -0.70710678118654),
        0.08838834764831844055010554526311,
        0.17677669529663688110021109052621
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