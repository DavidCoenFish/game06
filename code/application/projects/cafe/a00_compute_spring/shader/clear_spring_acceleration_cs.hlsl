cbuffer ConstantBuffer0 : register(b0)
{
    int _acceleration_count;
};

struct AccelerationData
{
    int3 acceleration;
};
RWStructuredBuffer<AccelerationData> g_acceleration_data : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 in_dispatch_thread_id : SV_DispatchThreadID, uint3 in_group_id : SV_GroupID)
{
    int acceleration_index = in_dispatch_thread_id.x + (in_dispatch_thread_id.y * 8);
    while (acceleration_index < _acceleration_count)
    {
        g_acceleration_data[acceleration_index].acceleration = int3(0, 0, 0);

        acceleration_index += 256;
    }
}