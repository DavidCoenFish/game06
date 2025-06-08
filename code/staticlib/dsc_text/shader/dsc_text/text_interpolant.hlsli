// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics
struct Interpolant
{
    /// [-1 ... 1]
    float4 _position : SV_Position;

    /// [0 ... 1]
    float2 _uv : TEXCOORD0;

    /// [0 ... 1] f4 to dot with texel sampled from glyph texture
    /// To allow different glyph on red, green, blue, alha chanels of glyph texture
    float4 _mask : COLOR0;

    /// [0 ... 1] colour of text
    float4 _colour : COLOR1;

};