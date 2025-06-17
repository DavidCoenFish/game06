#include "ui_panel_interpolant.hlsli"

struct Vertex
{
    // [-1 ... 1]
    float2 _position : POSITION;
    // [0 ... 1]
    float2 _uv_0 : TEXCOORD0;
    // [0 ... 1]
    float2 _uv_1 : TEXCOORD1;
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
    result._uv_0 = in_input._uv_0;
    result._uv_1 = in_input._uv_1;
    return result;
}
