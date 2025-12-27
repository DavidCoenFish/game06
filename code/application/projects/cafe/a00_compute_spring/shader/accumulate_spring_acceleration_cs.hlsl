cbuffer ConstantBuffer0 : register(b0)
{
    int _link_count;
    float _spring_constant;
};

struct LinkData
{
    int2 _link_index;
    float _length;
};
StructuredBuffer<LinkData> g_link_data : register(t0);

struct PositionData
{
    float3 _position;
};
StructuredBuffer<PositionData> g_pos_data : register(t1);

struct AccelerationData
{
    int3 acceleration;
};
RWStructuredBuffer<AccelerationData> g_acceleration_data : register(u0);

int3 ConvertFloat3ToInt3(float3 in_float3)
{
    return int3(
        ((int)(in_float3.x * 16777216.0)), //2 ^ 24
        ((int)(in_float3.y * 16777216.0)),
        ((int)(in_float3.z * 16777216.0))
        );
}

//F = -kx
int3 CalculateSpringForce(float3 in_pos_a, float3 in_pos_b, float in_length)
{
    float3 a_to_b = in_pos_b - in_pos_a;
    float a_to_b_length = length(a_to_b);
    float force = (a_to_b_length - in_length) * _spring_constant;
    float3 delta_float = a_to_b * (force / a_to_b_length);
    int3 delta_int = ConvertFloat3ToInt3(delta_float);
    return delta_int;
}

[numthreads(8, 8, 1)]
void main(uint3 in_dispatch_thread_id : SV_DispatchThreadID, uint3 in_group_id : SV_GroupID)
{
    int link_index = in_dispatch_thread_id.x + (in_dispatch_thread_id.y * 8);
    while (link_index < _link_count)
    {
        LinkData link_data = g_link_data[link_index];

        int pos_index_a = link_data._link_index[0];
        float3 pos_a = g_pos_data[pos_index_a]._position;

        int pos_index_b = link_data._link_index[1];
        float3 pos_b = g_pos_data[pos_index_b]._position;

        int3 delta = CalculateSpringForce(pos_a, pos_b, link_data._length);
        int3 delta_a = delta / 2;
        InterlockedAdd(g_acceleration_data[pos_index_a].acceleration.x, delta_a.x);
        InterlockedAdd(g_acceleration_data[pos_index_a].acceleration.y, delta_a.y);
        InterlockedAdd(g_acceleration_data[pos_index_a].acceleration.z, delta_a.z);

        int3 delta_b = delta_a - delta;
        InterlockedAdd(g_acceleration_data[pos_index_b].acceleration.x, delta_b.x);
        InterlockedAdd(g_acceleration_data[pos_index_b].acceleration.y, delta_b.y);
        InterlockedAdd(g_acceleration_data[pos_index_b].acceleration.z, delta_b.z);

        link_index += 256;
    }
}