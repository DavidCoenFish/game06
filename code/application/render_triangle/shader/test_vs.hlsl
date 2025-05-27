#include "test_interpolant.hlsli"

struct Vertex
{
    // [-1 ... 1]
    float2 _position : Position;
    // [0 ... 1]
    float4 _colour : COLOR1;
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
    result._colour = in_input._colour;
    return result;
}
