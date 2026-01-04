// want to be able to click and drag screen, and distort spring points appropriatly
#include "spring_acceleration.hlsli"

cbuffer ConstantBuffer0 : register(b0)
{
    float4 _click_pos_weight; // weight is zero if no mouse left button down / touch
    float4 _click_norm_range; // range is for dot product threashold? 
    int _pos_count;
};

struct PositionData
{
    float3 _position;
};
StructuredBuffer<PositionData> g_pos_data : register(t0);

struct AccelerationData
{
    int3 acceleration;
};
RWStructuredBuffer<AccelerationData> g_acceleration_data : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 in_dispatch_thread_id : SV_DispatchThreadID, uint3 in_group_id : SV_GroupID)
{
    int pos_index = in_dispatch_thread_id.x + (in_dispatch_thread_id.y * 8);
    while (pos_index < _pos_count)
    {
        PositionData pos_data = g_pos_data[pos_index];

        float3 to_pos = pos_data._position - _click_pos_weight.xyz;
        float3 to_pos_norm = normalize(to_pos);
        float between = 1.0 - dot(_click_norm_range.xyz, to_pos_norm);
        if (between < _click_norm_range.w)
        {
            float distance = dot(_click_norm_range.xyz, to_pos);
            float3 pos_on_click = _click_pos_weight.xyz + (_click_norm_range.xyz * distance);
            float3 delta = pos_on_click - pos_data._position;

            int3 delta_a = ConvertFloat3ToInt3(delta * _click_pos_weight.w);
            //int3 delta_a = ConvertFloat3ToInt3(float3(0.0f, 0.0f, 0.1f));
            InterlockedAdd(g_acceleration_data[pos_index].acceleration.x, delta_a.x);
            InterlockedAdd(g_acceleration_data[pos_index].acceleration.y, delta_a.y);
            InterlockedAdd(g_acceleration_data[pos_index].acceleration.z, delta_a.z);
        }

        pos_index += 256;
    }
}