#include "ui_panel_interpolant.hlsli"

struct Vertex
{
    float2 _position : POSITION;
    // so the rules for pixel shader interpolation of TEXCOORD0 are different to POSITION? at least if all 
    //float2 _uv : TEXCOORD0;
};

cbuffer ConstantBuffer : register(b0)
{
    float4 _pos_size;
    float4 _uv_size;
};

Interpolant main(Vertex in_input)
{
    Interpolant result;
    result._position = float4(
        _pos_size.x + (_pos_size.z * in_input._position.x),
        _pos_size.y - (_pos_size.w * in_input._position.y),
        0.0f, 
        1.0f
        );
    result._uv = float2(
        _uv_size.x + (_uv_size.z * in_input._position.x),
        _uv_size.y + (_uv_size.w * in_input._position.y)
        );
    return result;
}
