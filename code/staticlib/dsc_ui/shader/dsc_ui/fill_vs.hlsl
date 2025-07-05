#include "fill_interpolant.hlsli"

struct Vertex
{
    // [-1 ... 1]
    float2 _position : POSITION;
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
    return result;
}
