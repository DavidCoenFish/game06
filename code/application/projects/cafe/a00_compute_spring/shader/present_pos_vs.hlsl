#include "present_pos.hlsli"

struct Vertex
{
    int _position : POSITION;
};

cbuffer CameraConstantBuffer : register(b0)
{
    float4 _camera_pos_fov_horizontal;
    float4 _camera_at_fov_vertical;
    float4 _camera_up_camera_far;
};

struct PositionData
{
    float3 _position;
};
StructuredBuffer<PositionData> vertexUAV : register(t0);

float3 CameraProjection(float3 in_world_pos)
{
    float3 relative = in_world_pos - _camera_pos_fov_horizontal.xyz;
    float3 camera_right = cross(_camera_up_camera_far.xyz, _camera_at_fov_vertical.xyz);
    float depth = dot(_camera_at_fov_vertical.xyz, relative);

    float3 camera_relative = float3(
        depth,
        dot(_camera_up_camera_far.xyz, relative),
        dot(camera_right.xyz, relative));

    float r = length(camera_relative);
    float v = asin(camera_relative.y / r);
    float u = atan2(camera_relative.z, camera_relative.x);

    float3 screen = float3(
        u / (0.5 * _camera_pos_fov_horizontal.w),
        v / (0.5 * _camera_at_fov_vertical.w),
        depth / _camera_up_camera_far.w
        );

    return screen;
}

Interpolant main(Vertex in_input)
{
    Interpolant result;

    float3 pos = vertexUAV[in_input._position]._position;
    float3 screen = CameraProjection(pos.xyz);

    result._position = float4(
        screen.x,
        screen.y,
        screen.z,
        1.0f
        );

    return result;
}
