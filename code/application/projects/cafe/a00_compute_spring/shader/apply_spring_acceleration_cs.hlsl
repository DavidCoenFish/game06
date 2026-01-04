#include "spring_acceleration.hlsli"

cbuffer ConstantBuffer0 : register(b0)
{
    int _point_count;
    float _time_step;
    float _dampen;
};

struct PositionData
{
    float3 _position;
};
StructuredBuffer<PositionData> g_pos_data_prev_prev : register(t0);
StructuredBuffer<PositionData> g_pos_data_prev : register(t1);

struct AccelerationData
{
    int3 acceleration;
};
StructuredBuffer<AccelerationData> g_acceleration_data : register(t2);

RWStructuredBuffer<PositionData> g_pos_data : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 in_dispatch_thread_id : SV_DispatchThreadID, uint3 in_group_id : SV_GroupID)
{
    int pos_index = in_dispatch_thread_id.x + (in_dispatch_thread_id.y * 8);
    while (pos_index < _point_count)
    {
        float3 pos_prev_prev = g_pos_data_prev_prev[pos_index]._position;
        float3 pos_prev = g_pos_data_prev[pos_index]._position;
        int3 acceleration_int = g_acceleration_data[pos_index].acceleration;
        float3 acceleration = ConvertInt3ToFloat3(acceleration_int);

        // assume time between [pos_prev_prev, pos_prev] is _time_step, so we skip a /time_step, *time_step
        // _dampen is just to drain some energy out of the system, better could be to dampen more along the spring lenght, but spring may not be only the edges
        float3 new_velocity = ((pos_prev - pos_prev_prev) + (acceleration * _time_step)) * _dampen;
        float3 new_pos = pos_prev + new_velocity;

        g_pos_data[pos_index]._position = new_pos;

        pos_index += 256;
    }
}