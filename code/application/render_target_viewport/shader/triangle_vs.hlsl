#include "triangle_interpolant.hlsli"

struct Vertex
{
    // [-1 ... 1]
    float2 _position : POSITION;
    // [0 ... 1]
    float4 _colour : COLOR0;
};

cbuffer ConstantBufferBlock : register(b0)
{
    float4 _value;
};

Interpolant main(Vertex in_input)
{
    Interpolant result;
    result._position = float4(
        //in_input._position.x, 
        //in_input._position.x + (0.1 * _value[0]),
        (_value[0] * in_input._position.x) + (_value[1] * in_input._position.y),
        //in_input._position.y, 
        //in_input._position.y + (0.1 * _value[1]),
        (_value[2] * in_input._position.x) + (_value[3] * in_input._position.y),
        0.0f,
        1.0f
        );
    result._colour = in_input._colour;
    //result._colour = _value;
    //result._colour.a = 1.0;
    return result;
}
