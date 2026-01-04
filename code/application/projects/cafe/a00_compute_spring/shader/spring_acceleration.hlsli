float3 ConvertInt3ToFloat3(int3 in_int3)
{
    return float3(
        ((float)(in_int3.x)) / 16777216.0, //2 ^ 24
        ((float)(in_int3.y)) / 16777216.0,
        ((float)(in_int3.z)) / 16777216.0
        );
}

int3 ConvertFloat3ToInt3(float3 in_float3)
{
    return int3(
        ((int)(in_float3.x * 16777216.0)), //2 ^ 24
        ((int)(in_float3.y * 16777216.0)),
        ((int)(in_float3.z * 16777216.0))
        );
}

