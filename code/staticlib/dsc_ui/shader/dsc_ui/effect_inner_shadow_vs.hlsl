#include "effect_inner_shadow_interpolant.hlsli"

struct Vertex
{
    // [-1 ... 1]
    float2 _position : POSITION;
    // [0 ... 1]
    float2 _uv : TEXCOORD0;
};

Interpolant main(Vertex in_input)
{
    Interpolant result;
    result._position = float4(
        in_input._position.x,
        in_input._position.y,
        0.0f, 
        1.0f
        );
    result._uv = in_input._uv;
    return result;
}
