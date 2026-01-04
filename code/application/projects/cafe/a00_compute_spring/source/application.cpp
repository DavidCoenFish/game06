#include "application.h"

#include <dsc_camera/camera.h>
#include <dsc_common/data_helper.h>
#include <dsc_common/i_file_overlay.h>
#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>
#include <dsc_common/math.h>
#include <dsc_common/math_angle.h>
#include <dsc_common/vector_int2.h>
#include <dsc_common/vector_int3.h>
#include <dsc_common/vector_float3.h>
#include <dsc_onscreen_debug/onscreen_debug.h>
#include <dsc_onscreen_version/onscreen_version.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render_resource/frame.h>
#include <dsc_render_resource/geometry_generic.h>
#include <dsc_render_resource/constant_buffer.h>
#include <dsc_render_resource/constant_buffer_info.h>
#include <dsc_render_resource/shader.h>
#include <dsc_render_resource/shader_constant_buffer.h>
#include <dsc_render_resource/shader_pipeline_state_data.h>
#include <dsc_render_resource/shader_resource.h>
#include <dsc_render_resource/shader_resource_info.h>
#include <dsc_render_resource/unordered_access.h>
#include <dsc_render_resource/unordered_access_info.h>
#include <dsc_statistics/bookmark.h>
#include <dsc_statistics/bookmarks_per_second.h>
#include <dsc_statistics/value_int.h>
#include <dsc_statistics/event_store.h>
#include <dsc_statistics/i_event.h>
#include <dsc_text/text_manager.h>
#include <dsc_windows/window_helper.h>

namespace
{
    struct PositionData
    {
        float _position[3];
    };
    struct LinkData
    {
        int32 _index[2];
        float _length;
    };
    struct AccelerationData
    {
        int32 _acceleration[3];
    };

    struct TPresentPosConstantBuffer1
    {
        DscCommon::VectorFloat4 _colour;
    };

#if 0
    void GenerateTetrahedra(
        std::vector<uint8_t>& out_pos_data,
        std::vector<uint8_t>& out_link_data,
        std::vector<uint8_t>& out_acceleration_data,
        std::vector<uint8_t>& out_geometry_data,
        int32& out_pos_count,
        int32& out_link_count
    )
    {
        out_pos_count = 4;
        out_link_count = 6;

        DscCommon::DataHelper::AppendValue<float>(out_pos_data, -0.5f);
        DscCommon::DataHelper::AppendValue<float>(out_pos_data, 0.288675f);
        DscCommon::DataHelper::AppendValue<float>(out_pos_data, 0.0f);
        DscCommon::DataHelper::AppendValue<float>(out_pos_data, 0.5f);
        DscCommon::DataHelper::AppendValue<float>(out_pos_data, 0.288675f);
        DscCommon::DataHelper::AppendValue<float>(out_pos_data, 0.0f);
        DscCommon::DataHelper::AppendValue<float>(out_pos_data, 0.0f);
        DscCommon::DataHelper::AppendValue<float>(out_pos_data, -0.57735f);
        DscCommon::DataHelper::AppendValue<float>(out_pos_data, 0.0f);
        DscCommon::DataHelper::AppendValue<float>(out_pos_data, 0.0f);
        DscCommon::DataHelper::AppendValue<float>(out_pos_data, 0.0f);
        DscCommon::DataHelper::AppendValue<float>(out_pos_data, 1.5f);

        DscCommon::DataHelper::AppendValue<int32>(out_link_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_link_data, 1);
        DscCommon::DataHelper::AppendValue<float>(out_link_data, 1.0f);
        DscCommon::DataHelper::AppendValue<int32>(out_link_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_link_data, 2);
        DscCommon::DataHelper::AppendValue<float>(out_link_data, 1.0f);
        DscCommon::DataHelper::AppendValue<int32>(out_link_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_link_data, 3);
        DscCommon::DataHelper::AppendValue<float>(out_link_data, 1.0f);
        DscCommon::DataHelper::AppendValue<int32>(out_link_data, 1);
        DscCommon::DataHelper::AppendValue<int32>(out_link_data, 2);
        DscCommon::DataHelper::AppendValue<float>(out_link_data, 1.0f);
        DscCommon::DataHelper::AppendValue<int32>(out_link_data, 1);
        DscCommon::DataHelper::AppendValue<int32>(out_link_data, 3);
        DscCommon::DataHelper::AppendValue<float>(out_link_data, 1.0f);
        DscCommon::DataHelper::AppendValue<int32>(out_link_data, 2);
        DscCommon::DataHelper::AppendValue<int32>(out_link_data, 3);
        DscCommon::DataHelper::AppendValue<float>(out_link_data, 1.0f);

        DscCommon::DataHelper::AppendValue<int32>(out_acceleration_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_acceleration_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_acceleration_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_acceleration_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_acceleration_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_acceleration_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_acceleration_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_acceleration_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_acceleration_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_acceleration_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_acceleration_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_acceleration_data, 0);

#if 0 //points
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 1);
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 2);
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 3);
#else // links
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 1);
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 2);
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 0);
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 3);
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 1);
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 2);
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 1);
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 3);
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 2);
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, 3);
#endif
        return;
    }
#else

#define DSC_SQRT_3_DIV_SIX 0.28867513459481288225457439025098f
#define DSC_SQRT_2_3RD 0.81649658092772603273242802490196f
#define DSC_HALF_SQRT_3 0.86602540378443864676372317075294f

    const bool IsPointInsideSphere(
        const float in_radius_squared,
        const DscCommon::VectorFloat3& in_point
    )
    {
        const float distance_squared = DscCommon::Dot(in_point, in_point);
        return distance_squared <= in_radius_squared;
    }

    const DscCommon::VectorInt3 CalculateRange(
        const float in_radius,
        const float in_distance_between_points
    )
    {
        return DscCommon::VectorInt3(
            (int32)((in_radius / (in_distance_between_points)) + 1.5f),
            (int32)((in_radius / (in_distance_between_points * DSC_HALF_SQRT_3)) + 1.5f),
            (int32)((in_radius / (in_distance_between_points * DSC_SQRT_2_3RD)) + 1.5f)
        );
    }

    const DscCommon::VectorFloat3 CalculatePoint(const int32 in_index_x, const int32 in_index_y, const int32 in_index_z, const float in_distance_between_points)
    {
        return DscCommon::VectorFloat3(
            in_distance_between_points * ((float)in_index_x + (0.5f * (float)(in_index_y & 1)) - (0.5f * (float)(in_index_z & 1))),
            in_distance_between_points * (((float)in_index_y * DSC_HALF_SQRT_3) + (DSC_SQRT_3_DIV_SIX * (float)(in_index_z & 1))),
            in_distance_between_points * DSC_SQRT_2_3RD * (float)in_index_z
        );
    }

    const int32 MakeKey(const int32 in_index_x, const int32 in_index_y, const int32 in_index_z)
    {
        DSC_ASSERT((-1023 < in_index_x) && (in_index_x < 1024), "going to need a different hash");
        DSC_ASSERT((-1023 < in_index_y) && (in_index_y < 1024), "going to need a different hash");
        DSC_ASSERT((-1023 < in_index_z) && (in_index_z < 1024), "going to need a different hash");
        return (in_index_x & 0x3ff) | ((in_index_y << 10) & 0xFFC00) | ((in_index_z << 20) & 0x3FF00000);
    }

    void AttemptLink(
        int32& out_link_count,
        std::vector<uint8_t>& out_link_data,
        std::vector<uint8_t>& out_geometry_data,
        const float in_distance_between_points,
        const std::map<int32, int32>& in_map_index_hash_pos_index,
        const int32 in_a_x, const int32 in_a_y, const int32 in_a_z,
        const int32 in_b_x, const int32 in_b_y, const int32 in_b_z
    )
    {
        const int32 key_a = MakeKey(in_a_x, in_a_y, in_a_z);
        auto iter_a = in_map_index_hash_pos_index.find(key_a);
        if (in_map_index_hash_pos_index.end() == iter_a)
        {
            return;
        }
        const int32 pos_index_a = iter_a->second;

        const int32 key_b = MakeKey(in_b_x, in_b_y, in_b_z);
        auto iter_b = in_map_index_hash_pos_index.find(key_b);
        if (in_map_index_hash_pos_index.end() == iter_b)
        {
            return;
        }
        const int32 pos_index_b = iter_b->second;

        const DscCommon::VectorFloat3 point_a = CalculatePoint(in_a_x, in_a_y, in_a_z, in_distance_between_points);
        const DscCommon::VectorFloat3 point_b = CalculatePoint(in_b_x, in_b_y, in_b_z, in_distance_between_points);

        const float length = DscCommon::Length(point_a - point_b);

        DscCommon::DataHelper::AppendValue<int32>(out_link_data, pos_index_a);
        DscCommon::DataHelper::AppendValue<int32>(out_link_data, pos_index_b);
        DscCommon::DataHelper::AppendValue<float>(out_link_data, length);

        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, pos_index_a);
        DscCommon::DataHelper::AppendValue<int32>(out_geometry_data, pos_index_b);

        out_link_count += 1;
    }

    void GenerateSphere(
        const float in_radius,
        const float in_distance_between_points,
        std::vector<uint8_t>& out_pos_data,
        std::vector<uint8_t>& out_link_data,
        std::vector<uint8_t>& out_acceleration_data,
        std::vector<uint8_t>& out_geometry_data,
        int32& out_pos_count,
        int32& out_link_count
    )
    {
        const float radius_squared = in_radius * in_radius;
        const DscCommon::VectorInt3 range = CalculateRange(in_radius, in_distance_between_points);

        out_pos_count = 0;
        out_link_count = 0;

        // populate pos
        std::map<int32, int32> map_index_hash_pos_index;
        for (int32 index_z = -(range.GetZ()); index_z <= range.GetZ(); ++index_z)
        {
            for (int32 index_y = -(range.GetY()); index_y <= range.GetY(); ++index_y)
            {
                for (int32 index_x = -(range.GetX()); index_x <= range.GetX(); ++index_x)
                {
                    const int32 key = MakeKey(index_x, index_y, index_z);
                    const DscCommon::VectorFloat3 point = CalculatePoint(index_x, index_y, index_z, in_distance_between_points);
                    if (false == IsPointInsideSphere(radius_squared, point))
                    {
                        continue;
                    }

                    float noise_x = 0.1f * (float)(index_x & 1);
                    float noise_y = 0.2f * (float)(index_y & 1);
                    float noise_z = 0.3f * (float)(index_z & 1);

                    map_index_hash_pos_index[key] = out_pos_count;
                    DscCommon::DataHelper::AppendValue<float>(out_pos_data, point.GetX() + noise_x);
                    DscCommon::DataHelper::AppendValue<float>(out_pos_data, point.GetY() + noise_y);
                    DscCommon::DataHelper::AppendValue<float>(out_pos_data, point.GetZ() + noise_z);
                    out_pos_count += 1;

                    DscCommon::DataHelper::AppendValue<int32>(out_acceleration_data, 0);
                    DscCommon::DataHelper::AppendValue<int32>(out_acceleration_data, 0);
                    DscCommon::DataHelper::AppendValue<int32>(out_acceleration_data, 0);
                }
            }
        }

        // link data
        for (int32 index_z = -(range.GetZ()); index_z <= range.GetZ(); ++index_z)
        {
            for (int32 index_y = -(range.GetY()); index_y <= range.GetY(); ++index_y)
            {
                for (int32 index_x = -(range.GetX()); index_x <= range.GetX(); ++index_x)
                {
                    // try to link to the right
                    AttemptLink(
                        out_link_count,
                        out_link_data,
                        out_geometry_data,
                        in_distance_between_points,
                        map_index_hash_pos_index,
                        index_x, index_y, index_z, 
                        index_x + 1, index_y, index_z
                    );
                    // to top right
                    AttemptLink(
                        out_link_count,
                        out_link_data,
                        out_geometry_data,
                        in_distance_between_points,
                        map_index_hash_pos_index,
                        index_x, index_y, index_z,
                        index_x + (index_y & 1), index_y + 1, index_z
                    );

                    // to top left
                    AttemptLink(
                        out_link_count,
                        out_link_data,
                        out_geometry_data,
                        in_distance_between_points,
                        map_index_hash_pos_index,
                        index_x, index_y, index_z,
                        index_x + (index_y & 1) - 1, index_y + 1, index_z
                    );

                    // to the three above
                    if (0 == (index_z & 1))
                    {
                        AttemptLink(
                            out_link_count,
                            out_link_data,
                            out_geometry_data,
                            in_distance_between_points,
                            map_index_hash_pos_index,
                            index_x, index_y, index_z,
                            index_x + 1, index_y, index_z + 1
                        );
                        AttemptLink(
                            out_link_count,
                            out_link_data,
                            out_geometry_data,
                            in_distance_between_points,
                            map_index_hash_pos_index,
                            index_x, index_y, index_z,
                            index_x, index_y, index_z + 1
                        );
                        AttemptLink(
                            out_link_count,
                            out_link_data,
                            out_geometry_data,
                            in_distance_between_points,
                            map_index_hash_pos_index,
                            index_x, index_y, index_z,
                            index_x + (index_y & 1), index_y - 1, index_z + 1
                        );
                    }
                    else
                    {
                        AttemptLink(
                            out_link_count,
                            out_link_data,
                            out_geometry_data,
                            in_distance_between_points,
                            map_index_hash_pos_index,
                            index_x, index_y, index_z,
                            index_x, index_y, index_z + 1
                        );
                        AttemptLink(
                            out_link_count,
                            out_link_data,
                            out_geometry_data,
                            in_distance_between_points,
                            map_index_hash_pos_index,
                            index_x, index_y, index_z,
                            index_x - 1, index_y, index_z + 1
                        );
                        AttemptLink(
                            out_link_count,
                            out_link_data,
                            out_geometry_data,
                            in_distance_between_points,
                            map_index_hash_pos_index,
                            index_x, index_y, index_z,
                            index_x - 1 + (index_y & 1), index_y + 1, index_z + 1
                        );
                    }
                }
            }
        }

    }
#endif
}

Application::Resources::Resources() 
{
    //nop
}

Application::Application(const HWND in_hwnd, const bool in_fullScreen, const int in_defaultWidth, const int in_defaultHeight)
    : DscWindows::IWindowApplication(in_hwnd, in_fullScreen, in_defaultWidth, in_defaultHeight)
{
    std::vector<uint8_t> pos_data;
    std::vector<uint8_t> link_data;
    std::vector<uint8_t> acceleration_data;
    std::vector<uint8_t> geometry_data;
    int32 pos_count = 0;
    int32 link_count = 0;

#if 0
    GenerateTetrahedra(
        pos_data,
        link_data,
        acceleration_data,
        geometry_data,
        pos_count,
        link_count
    );
#else
    GenerateSphere(
        5.0f,
        0.75f,
        pos_data,
        link_data,
        acceleration_data,
        geometry_data,
        pos_count,
        link_count
    );

#endif

    _clear_spring_acceleration_constant_buffer._acceleration_count = pos_count;

    _accumulate_spring_acceleration_constant_buffer._link_count = link_count;
    _accumulate_spring_acceleration_constant_buffer._spring_constant = 1.0f;

    _apply_spring_acceleration_constant_buffer._point_count = pos_count;
    _apply_spring_acceleration_constant_buffer._time_step = 1.0f / (60.0f * 40.0f);
    _apply_spring_acceleration_constant_buffer._dampen = 0.995f;

    _click_spring_acceleration_constant_buffer._pos_count = pos_count;

    _file_system = std::make_unique<DscCommon::FileSystem>();
    _draw_system = DscRender::DrawSystem::FactoryClearColour(in_hwnd, DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 0.0f));
    //_draw_system = DscRender::DrawSystem::FactoryClearColour(in_hwnd, DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.0f));
    _event_store = std::make_unique<DscStatistics::EventStore>();
    if (nullptr != _event_store)
    {
        _event_store->SetStoredEvents("FPS");
        _event_store->SetStoredEvents("pos_count");
        _event_store->SetStoredEvents("link_count");

        _event_store->AddEventDerrived(
            std::make_unique<DscStatistics::BookmarksPerSecond>("FPS"),
            std::vector<std::string>({"frame"})
            );
    }

    _resources = std::make_unique<Resources>();
    if ((nullptr != _file_system) && (nullptr != _draw_system))
    {
        _resources->_text_manager = std::make_unique<DscText::TextManager>(*_draw_system, *_file_system);
        _resources->_onscreen_debug = std::make_unique<DscOnscreenDebug::OnscreenDebug>(*_draw_system, *_file_system, *(_resources->_text_manager));
        _resources->_onscreen_version = std::make_unique<DscOnscreenVersion::OnscreenVersion>(*_draw_system, *_file_system, *(_resources->_text_manager));
        _resources->_camera = std::make_unique<DscCamera::Camera>(
            DscCommon::VectorFloat3(0.0f, 20.0f, 0.0f), //in_initial_pos, // 1.0f, 20.0f, 1.0f
            DscCommon::VectorFloat3(0.0f, -1.0f, 0.0f), //in_initial_at,
            DscCommon::VectorFloat3(0.0f, 0.0f, 1.0f), //in_initial_up,
            30.0f, //in_far,
            90.0f, //in_target_vertical_fov,
            DscCommon::VectorInt2(in_defaultWidth, in_defaultHeight)
            );
    }

    DSC_LOG_DIAGNOSTIC(LOG_TOPIC_APPLICATION, "pos_count:%d, link_count:%d\n", pos_count, link_count);
    if (nullptr != _event_store)
    {
        _event_store->AddEvent(std::make_unique<DscStatistics::ValueInt>("pos_count", pos_count));
        _event_store->AddEvent(std::make_unique<DscStatistics::ValueInt>("link_count", link_count));
    }

    //_pos_data
    {
        D3D12_RESOURCE_DESC desc = {
            D3D12_RESOURCE_DIMENSION_BUFFER, //D3D12_RESOURCE_DIMENSION Dimension;
            D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, //UINT64 Alignment;
            pos_data.size(), //UINT64 Width;
            1, //UINT Height;
            1, //UINT16 DepthOrArraySize;
            1, //UINT16 MipLevels;
            DXGI_FORMAT_UNKNOWN, //DXGI_FORMAT Format;
            DXGI_SAMPLE_DESC{ 1, 0 }, //DXGI_SAMPLE_DESC SampleDesc;
            D3D12_TEXTURE_LAYOUT_ROW_MAJOR, //D3D12_TEXTURE_LAYOUT Layout;
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS //D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE //D3D12_RESOURCE_FLAGS Flags;
        };
        // Describe and create a UAV for the resource.
        D3D12_UNORDERED_ACCESS_VIEW_DESC unordered_access_view_desc({});
        unordered_access_view_desc.Format = DXGI_FORMAT_UNKNOWN;
        unordered_access_view_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        unordered_access_view_desc.Buffer.StructureByteStride = sizeof(PositionData);
        unordered_access_view_desc.Buffer.CounterOffsetInBytes = 0;
        unordered_access_view_desc.Buffer.FirstElement = 0;
        unordered_access_view_desc.Buffer.NumElements = static_cast<UINT>(pos_data.size() / sizeof(PositionData));
        unordered_access_view_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

        // Describe and create a SRV for the resource.
        D3D12_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc({});
        shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;
        shader_resource_view_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        shader_resource_view_desc.Buffer.StructureByteStride = sizeof(PositionData);
        shader_resource_view_desc.Buffer.FirstElement = 0;
        shader_resource_view_desc.Buffer.NumElements = static_cast<UINT>(pos_data.size() / sizeof(PositionData));
        shader_resource_view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        for (int32 index = 0; index < k_pos_data_count; ++index)
        {
            _resources->_pos_data[index] = std::make_shared<DscRenderResource::UnorderedAccess>(
                _draw_system.get(),
                _draw_system->MakeHeapWrapperCbvSrvUav(),
                //nullptr, //_draw_system->MakeHeapWrapperCbvSrvUavNone(),
                _draw_system->MakeHeapWrapperCbvSrvUav(),
                desc, //const D3D12_RESOURCE_DESC & in_desc,
                unordered_access_view_desc,
                pos_data, //const std::vector<uint8_t>&in_data
                shader_resource_view_desc
                );
        }
    }

    // present
    {
        std::vector < D3D12_INPUT_ELEMENT_DESC > input_element_desc_array;
        input_element_desc_array.push_back(D3D12_INPUT_ELEMENT_DESC
            {
                "POSITION", 0, DXGI_FORMAT_R32_SINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
                    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
            });
        // present geom
        {
            _resources->_geometry = std::make_shared<DscRenderResource::GeometryGeneric>(
                _draw_system.get(),
                D3D_PRIMITIVE_TOPOLOGY_LINELIST,
                input_element_desc_array,
                geometry_data,
                1
                );
        }
        // present shader
        {
            std::vector<uint8> vertex_shader_data;
            if (false == _file_system->LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "present_pos_vs.cso")))
            {
                DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "failed to load vertex shader\n");
            }

            std::vector<uint8> pixel_shader_data;
            if (false == _file_system->LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "present_pos_ps.cso")))
            {
                DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "failed to load pixel shader\n");
            }

            std::vector < DXGI_FORMAT > render_target_format;
            render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);

            DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
                input_element_desc_array,
                D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
                DXGI_FORMAT_UNKNOWN,
                render_target_format,
                DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),  //CD3DX12_BLEND_DESC(D3D12_DEFAULT),
                //DscRenderResource::ShaderPipelineStateData::FactoryBlendDesc(),  //CD3DX12_BLEND_DESC(D3D12_DEFAULT),
                CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
                CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
            );

            std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
            array_shader_resource_info.push_back(
                DscRenderResource::ShaderResourceInfo::FactoryNoSampler(
                    nullptr, //_resources->_pos_data->GetShaderViewHeapWrapperItem(),
                    D3D12_SHADER_VISIBILITY_VERTEX
                )
            );

            std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
            array_shader_constants_info.push_back(
                DscRenderResource::ConstantBufferInfo::Factory(
                    DscCamera::Camera::TConstantBuffer(),
                    D3D12_SHADER_VISIBILITY_VERTEX
                )
            );

            TPresentPosConstantBuffer1 colour_constant_buffer = {};
            colour_constant_buffer._colour.Set(1.0f, 1.0f, 1.0f, 1.0f);
            array_shader_constants_info.push_back(
                DscRenderResource::ConstantBufferInfo::Factory(
                    colour_constant_buffer,
                    D3D12_SHADER_VISIBILITY_PIXEL
                )
            );

            std::vector<std::shared_ptr<DscRenderResource::UnorderedAccessInfo>> array_unordered_access_info;

            _resources->_present_pos_shader = std::make_shared<DscRenderResource::Shader>(
                _draw_system.get(),
                shader_pipeline_state_data,
                vertex_shader_data,
                std::vector<uint8_t>(),
                pixel_shader_data,
                array_shader_resource_info,
                array_shader_constants_info,
                std::vector<uint8_t>(),
                array_unordered_access_info
                );

            for (int32 index = 0; index < k_pos_data_count; ++index)
            {
                _resources->_present_pos_constant_buffer[index] = _resources->_present_pos_shader->MakeShaderConstantBuffer(_draw_system.get());
            }
        }
    }

    // _link_data
    {
        D3D12_RESOURCE_DESC desc = {
            D3D12_RESOURCE_DIMENSION_BUFFER, //D3D12_RESOURCE_DIMENSION Dimension;
            D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, //UINT64 Alignment;
            link_data.size(), //UINT64 Width;
            1, //UINT Height;
            1, //UINT16 DepthOrArraySize;
            1, //UINT16 MipLevels;
            DXGI_FORMAT_UNKNOWN, //DXGI_FORMAT Format;
            DXGI_SAMPLE_DESC{ 1, 0 }, //DXGI_SAMPLE_DESC SampleDesc;
            D3D12_TEXTURE_LAYOUT_ROW_MAJOR, //D3D12_TEXTURE_LAYOUT Layout;
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS //D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE //D3D12_RESOURCE_FLAGS Flags;
        };
        // Describe and create a SRV for the texture.
        D3D12_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc({});
        shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;
        shader_resource_view_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER; // D3D12_UAV_DIMENSION_TEXTURE2D;
        shader_resource_view_desc.Buffer.StructureByteStride = sizeof(LinkData);
        shader_resource_view_desc.Buffer.FirstElement = 0;
        shader_resource_view_desc.Buffer.NumElements = static_cast<UINT>(link_data.size() / sizeof(LinkData));
        shader_resource_view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        // only used as ShaderResource, (t)
        _resources->_link_data = std::make_shared<DscRenderResource::UnorderedAccess>(
            _draw_system.get(),
            nullptr, //_draw_system->MakeHeapWrapperCbvSrvUav(),
            //nullptr, //_draw_system->MakeHeapWrapperCbvSrvUavNone(),
            _draw_system->MakeHeapWrapperCbvSrvUav(),
            desc, //const D3D12_RESOURCE_DESC & in_desc,
            D3D12_UNORDERED_ACCESS_VIEW_DESC(),
            link_data, //const std::vector<uint8_t>&in_data
            shader_resource_view_desc
            );
    }

    // _acceleration_data
    {
        D3D12_RESOURCE_DESC desc = {
            D3D12_RESOURCE_DIMENSION_BUFFER, //D3D12_RESOURCE_DIMENSION Dimension;
            D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, //UINT64 Alignment;
            acceleration_data.size(), //UINT64 Width;
            1, //UINT Height;
            1, //UINT16 DepthOrArraySize;
            1, //UINT16 MipLevels;
            DXGI_FORMAT_UNKNOWN, //DXGI_FORMAT Format;
            DXGI_SAMPLE_DESC{ 1, 0 }, //DXGI_SAMPLE_DESC SampleDesc;
            D3D12_TEXTURE_LAYOUT_ROW_MAJOR, //D3D12_TEXTURE_LAYOUT Layout;
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS //D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE //D3D12_RESOURCE_FLAGS Flags;
        };

        // Describe and create a UAV for the resource.
        D3D12_UNORDERED_ACCESS_VIEW_DESC unordered_access_view_desc({});
        unordered_access_view_desc.Format = DXGI_FORMAT_UNKNOWN;
        unordered_access_view_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        unordered_access_view_desc.Buffer.StructureByteStride = sizeof(AccelerationData);
        unordered_access_view_desc.Buffer.CounterOffsetInBytes = 0;
        unordered_access_view_desc.Buffer.FirstElement = 0;
        unordered_access_view_desc.Buffer.NumElements = static_cast<UINT>(acceleration_data.size() / sizeof(AccelerationData));
        unordered_access_view_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

        D3D12_UNORDERED_ACCESS_VIEW_DESC clear_view_desc({});
        clear_view_desc.Format = DXGI_FORMAT_R32_UINT;
        clear_view_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
        clear_view_desc.Texture1D.MipSlice = 0;

        // Describe and create a SRV for the texture.
        D3D12_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc({});
        shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;
        shader_resource_view_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER; // D3D12_UAV_DIMENSION_TEXTURE2D;
        shader_resource_view_desc.Buffer.StructureByteStride = sizeof(AccelerationData);
        shader_resource_view_desc.Buffer.FirstElement = 0;
        shader_resource_view_desc.Buffer.NumElements = static_cast<UINT>(acceleration_data.size() / sizeof(AccelerationData));
        shader_resource_view_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        _resources->_acceleration_data = std::make_shared<DscRenderResource::UnorderedAccess>(
            _draw_system.get(),
            _draw_system->MakeHeapWrapperCbvSrvUav(),
            //_draw_system->MakeHeapWrapperCbvSrvUavNone(),
            _draw_system->MakeHeapWrapperCbvSrvUav(),
            desc, //const D3D12_RESOURCE_DESC & in_desc,
            unordered_access_view_desc,
            acceleration_data, //const std::vector<uint8_t>&in_data
            shader_resource_view_desc,
            clear_view_desc
            );
    }

    // _clear_spring_acceleration_shader;
    if ((nullptr != _file_system) && (nullptr != _draw_system))
    {
        std::vector<uint8> compute_shader_data;
        if (false == _file_system->LoadFile(compute_shader_data, DscCommon::FileSystem::JoinPath("shader", "clear_spring_acceleration_cs.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "failed to load compute shader\n");
        }
        DscRenderResource::ShaderPipelineStateData compute_pipeline_state_data = DscRenderResource::ShaderPipelineStateData::FactoryComputeShader();

        std::vector< std::shared_ptr< DscRenderResource::ConstantBufferInfo > > array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(_clear_spring_acceleration_constant_buffer)
        );

        std::vector< std::shared_ptr< DscRenderResource::ShaderResourceInfo > > array_shader_resource_info;

        std::vector< std::shared_ptr< DscRenderResource::UnorderedAccessInfo > > array_unordered_access_info;
        array_unordered_access_info.push_back(
            DscRenderResource::UnorderedAccessInfo::Factory(
                _resources->_acceleration_data->GetHeapWrapperItem()
            ));

        _resources->_clear_spring_acceleration_shader = std::make_shared<DscRenderResource::Shader>(
            _draw_system.get(),
            compute_pipeline_state_data,
            std::vector<uint8_t>(),
            std::vector<uint8_t>(),
            std::vector<uint8_t>(),
            array_shader_resource_info,
            array_shader_constants_info,
            compute_shader_data,
            array_unordered_access_info
            );

        _resources->_clear_spring_acceleration_constant_buffer = _resources->_clear_spring_acceleration_shader->MakeShaderConstantBuffer(_draw_system.get());
    }

    // _accumulate_spring_acceleration_shader;
    if ((nullptr != _file_system) && (nullptr != _draw_system))
    {
        std::vector<uint8> compute_shader_data;
        if (false == _file_system->LoadFile(compute_shader_data, DscCommon::FileSystem::JoinPath("shader", "accumulate_spring_acceleration_cs.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "failed to load compute shader\n");
        }
        DscRenderResource::ShaderPipelineStateData compute_pipeline_state_data = DscRenderResource::ShaderPipelineStateData::FactoryComputeShader();

        std::vector< std::shared_ptr< DscRenderResource::ConstantBufferInfo > > array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(_accumulate_spring_acceleration_constant_buffer)
        );

        std::vector< std::shared_ptr< DscRenderResource::ShaderResourceInfo > > array_shader_resource_info;
        array_shader_resource_info.push_back(DscRenderResource::ShaderResourceInfo::FactoryNoSampler(
            _resources->_link_data->GetShaderViewHeapWrapperItem()
            ));
        array_shader_resource_info.push_back(DscRenderResource::ShaderResourceInfo::FactoryNoSampler(
            nullptr
            ));

        std::vector< std::shared_ptr< DscRenderResource::UnorderedAccessInfo > > array_unordered_access_info;
        array_unordered_access_info.push_back(
            DscRenderResource::UnorderedAccessInfo::Factory(
                _resources->_acceleration_data->GetHeapWrapperItem()
            ));

        _resources->_accumulate_spring_acceleration_shader = std::make_shared<DscRenderResource::Shader>(
            _draw_system.get(),
            compute_pipeline_state_data,
            std::vector<uint8_t>(),
            std::vector<uint8_t>(),
            std::vector<uint8_t>(),
            array_shader_resource_info,
            array_shader_constants_info,
            compute_shader_data,
            array_unordered_access_info
        );

        _resources->_accumulate_spring_acceleration_constant_buffer = _resources->_accumulate_spring_acceleration_shader->MakeShaderConstantBuffer(_draw_system.get());
    }

    //_click_spring_acceleration_shader
    if ((nullptr != _file_system) && (nullptr != _draw_system))
    {
        std::vector<uint8> compute_shader_data;
        if (false == _file_system->LoadFile(compute_shader_data, DscCommon::FileSystem::JoinPath("shader", "click_spring_acceleration_cs.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "failed to load compute shader\n");
        }
        DscRenderResource::ShaderPipelineStateData compute_pipeline_state_data = DscRenderResource::ShaderPipelineStateData::FactoryComputeShader();

        std::vector< std::shared_ptr< DscRenderResource::ConstantBufferInfo > > array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(_click_spring_acceleration_constant_buffer)
        );

        std::vector< std::shared_ptr< DscRenderResource::ShaderResourceInfo > > array_shader_resource_info;
        array_shader_resource_info.push_back(DscRenderResource::ShaderResourceInfo::FactoryNoSampler(
            nullptr
        ));

        std::vector< std::shared_ptr< DscRenderResource::UnorderedAccessInfo > > array_unordered_access_info;
        array_unordered_access_info.push_back(
            DscRenderResource::UnorderedAccessInfo::Factory(
                _resources->_acceleration_data->GetHeapWrapperItem()
            ));

        _resources->_click_spring_acceleration_shader = std::make_shared<DscRenderResource::Shader>(
            _draw_system.get(),
            compute_pipeline_state_data,
            std::vector<uint8_t>(),
            std::vector<uint8_t>(),
            std::vector<uint8_t>(),
            array_shader_resource_info,
            array_shader_constants_info,
            compute_shader_data,
            array_unordered_access_info
            );
        _resources->_click_spring_acceleration_constant_buffer = _resources->_click_spring_acceleration_shader->MakeShaderConstantBuffer(_draw_system.get());
    }

    // _apply_spring_acceleration_shader;
    if ((nullptr != _file_system) && (nullptr != _draw_system))
    {
        std::vector<uint8> compute_shader_data;
        if (false == _file_system->LoadFile(compute_shader_data, DscCommon::FileSystem::JoinPath("shader", "apply_spring_acceleration_cs.cso")))
        {
            DSC_LOG_WARNING(LOG_TOPIC_APPLICATION, "failed to load compute shader\n");
        }
        DscRenderResource::ShaderPipelineStateData compute_pipeline_state_data = DscRenderResource::ShaderPipelineStateData::FactoryComputeShader();

        std::vector< std::shared_ptr< DscRenderResource::ConstantBufferInfo > > array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(_apply_spring_acceleration_constant_buffer)
        );

        std::vector< std::shared_ptr< DscRenderResource::ShaderResourceInfo > > array_shader_resource_info;
        array_shader_resource_info.push_back(DscRenderResource::ShaderResourceInfo::FactoryNoSampler(
            nullptr
        ));
        array_shader_resource_info.push_back(DscRenderResource::ShaderResourceInfo::FactoryNoSampler(
            nullptr
        ));
        array_shader_resource_info.push_back(DscRenderResource::ShaderResourceInfo::FactoryNoSampler(
            _resources->_acceleration_data->GetShaderViewHeapWrapperItem()
        ));

        std::vector< std::shared_ptr< DscRenderResource::UnorderedAccessInfo > > array_unordered_access_info;
        array_unordered_access_info.push_back(
            DscRenderResource::UnorderedAccessInfo::Factory(
                nullptr
            ));

        _resources->_apply_spring_acceleration_shader = std::make_shared<DscRenderResource::Shader>(
            _draw_system.get(),
            compute_pipeline_state_data,
            std::vector<uint8_t>(),
            std::vector<uint8_t>(),
            std::vector<uint8_t>(),
            array_shader_resource_info,
            array_shader_constants_info,
            compute_shader_data,
            array_unordered_access_info
            );
        _resources->_apply_spring_acceleration_constant_buffer = _resources->_apply_spring_acceleration_shader->MakeShaderConstantBuffer(_draw_system.get());
    }
}

Application::~Application()
{
    if (_draw_system)
    {
        _draw_system->WaitForGpu();
    }

    _resources.reset();

    _draw_system.reset();
    _file_system.reset();
}

const bool Application::Update()
{
    BaseType::Update();

    if (nullptr != _event_store)
    {
        _event_store->AddEvent(std::make_unique<DscStatistics::Bookmark>("frame", std::chrono::system_clock::now()));
    }

    // do we need to actually check focus of app
    const bool window_focused = false == GetMinimized();

    if (_resources && _resources->_camera)
    {
        _resources->_camera->Update(GetHwnd(), window_focused);
    }

    bool click = false;
    DscCommon::VectorInt2 click_pos;

    //bool _prev_mouse_pos_valid = false;
    //DscCommon::VectorInt2 _prev_mouse_pos;
    {
        bool left_button = false;
        bool right_button = false;

        // top left is 0,0 and +x is across the screen, +y is down the screen
        const bool mouse_valid = DscWindows::GetMouseState(GetHwnd(), click_pos, left_button, right_button);

        if (mouse_valid && right_button)
        {
            click = true;
        }
    }

    if (_draw_system && (false == GetMinimized()))
    {
        std::unique_ptr<DscRenderResource::Frame> frame = DscRenderResource::Frame::CreateNewFrame(*_draw_system);

        for (int32 index = 0; index < 40; ++index)
        {
            StepPosDataIndex();

            // clear acceleration
            frame->ResourceBarrier(_resources->_acceleration_data.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            _resources->_clear_spring_acceleration_shader->SetUnorderedAccessViewHandle(
                0, _resources->_acceleration_data->GetHeapWrapperItem()
            );
            frame->SetShader(_resources->_clear_spring_acceleration_shader, _resources->_clear_spring_acceleration_constant_buffer);
            frame->Dispatch(8, 8);

            // update acceleration
            //   set T1 (pos prev)
            {
                DscRenderResource::UnorderedAccess* pos_data = _resources->_pos_data[GetPosDataIndexPrev()].get();
                frame->ResourceBarrier(pos_data, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                _resources->_accumulate_spring_acceleration_shader->SetShaderResourceViewHandle(
                    1, pos_data->GetShaderViewHeapWrapperItem()
                );
            }
            //   set U0 (acceleration)
            frame->ResourceBarrier(_resources->_acceleration_data.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            _resources->_accumulate_spring_acceleration_shader->SetUnorderedAccessViewHandle(
                0, _resources->_acceleration_data->GetHeapWrapperItem()
            );
            //   dispatch
            frame->SetShader(_resources->_accumulate_spring_acceleration_shader, _resources->_accumulate_spring_acceleration_constant_buffer);
            frame->Dispatch(8, 8);

            if (click && _resources && _resources->_camera)
            {
                // Set T0 position
                {
                    DscRenderResource::UnorderedAccess* pos_data = _resources->_pos_data[GetPosDataIndexPrev()].get();
                    frame->ResourceBarrier(pos_data, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                    _resources->_click_spring_acceleration_shader->SetShaderResourceViewHandle(
                        0, pos_data->GetShaderViewHeapWrapperItem()
                    );
                }

                // Set U0 acceleration
                frame->ResourceBarrier(_resources->_acceleration_data.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                _resources->_click_spring_acceleration_shader->SetUnorderedAccessViewHandle(
                    0, _resources->_acceleration_data->GetHeapWrapperItem()
                );

                // Set constant buffer b0
                {
                    const DscCamera::Camera::TConstantBuffer& camera_buffer = _resources->_camera->GetConstantBuffer();

                    _click_spring_acceleration_constant_buffer._click_pos_weight[0] = camera_buffer._camera_pos_fov_horizontal[0];
                    _click_spring_acceleration_constant_buffer._click_pos_weight[1] = camera_buffer._camera_pos_fov_horizontal[1];
                    _click_spring_acceleration_constant_buffer._click_pos_weight[2] = camera_buffer._camera_pos_fov_horizontal[2];
                    _click_spring_acceleration_constant_buffer._click_pos_weight[3] = 0.075f;

                    _click_spring_acceleration_constant_buffer._click_norm_range[0] = 0.0f;
                    _click_spring_acceleration_constant_buffer._click_norm_range[1] = -1.0f;
                    _click_spring_acceleration_constant_buffer._click_norm_range[2] = 0.0f;
                    _click_spring_acceleration_constant_buffer._click_norm_range[3] = 0.002f; // 0.034f; // 1.0 - cos(15deg)

                    TClickSpringAccelerationConstantBuffer& buffer = _resources->_click_spring_acceleration_constant_buffer->GetConstant<TClickSpringAccelerationConstantBuffer>(0);
                    buffer = _click_spring_acceleration_constant_buffer;
                }

                //   dispatch
                frame->SetShader(_resources->_click_spring_acceleration_shader, _resources->_click_spring_acceleration_constant_buffer);
                frame->Dispatch(8, 8);
            }

            // apply spring
            //   set T0 (pos prev prev)
            {
                DscRenderResource::UnorderedAccess* pos_data = _resources->_pos_data[GetPosDataIndexPrevPrev()].get();
                frame->ResourceBarrier(pos_data, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                _resources->_apply_spring_acceleration_shader->SetShaderResourceViewHandle(
                    0, pos_data->GetShaderViewHeapWrapperItem()
                );
            }
            //   set T1 (pos prev)
            {
                DscRenderResource::UnorderedAccess* pos_data = _resources->_pos_data[GetPosDataIndexPrev()].get();
                frame->ResourceBarrier(pos_data, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                _resources->_apply_spring_acceleration_shader->SetShaderResourceViewHandle(
                    1, pos_data->GetShaderViewHeapWrapperItem()
                );
            }
            //   set T2 (acceleration)
            {
                DscRenderResource::UnorderedAccess* acceleration_data = _resources->_acceleration_data.get();
                frame->ResourceBarrier(acceleration_data, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                _resources->_apply_spring_acceleration_shader->SetShaderResourceViewHandle(
                    2, acceleration_data->GetShaderViewHeapWrapperItem()
                );
            }
            //   set U0 (pos)
            {
                DscRenderResource::UnorderedAccess* pos_data = _resources->_pos_data[GetPosDataIndex()].get();
                frame->ResourceBarrier(pos_data, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                _resources->_apply_spring_acceleration_shader->SetUnorderedAccessViewHandle(
                    0, pos_data->GetHeapWrapperItem()
                );
            }

            //   dispatch
            frame->SetShader(_resources->_apply_spring_acceleration_shader, _resources->_apply_spring_acceleration_constant_buffer);
            frame->Dispatch(8, 8);
        }

        frame->SetRenderTarget(_draw_system->GetRenderTargetBackBuffer(), true);

        // display is tearing a bit funny, stepping through each frame, the last drawn is dominant, but letting it just run, things look green
        // also now the compute advanced 40 times a frame, the tripple buffer lag is a lot less, so only draw the current pos data
        PresentPos(frame, GetPosDataIndex(), DscCommon::VectorFloat4(1.0f, 1.0f, 1.0f, 1.0f));

        UpdateDebugText();

        if (_resources->_onscreen_debug)
        {
            _resources->_onscreen_debug->Update(*_draw_system, *frame, *_resources->_text_manager, false);
        }

        if (_resources->_onscreen_version)
        {
            _resources->_onscreen_version->Update(*_draw_system, *frame, *_resources->_text_manager, false);
        }
    }

    return true;
}

void Application::OnKey(const int in_vk_code, const int in_scan_code, const bool in_repeat_flag, const int in_repeat_count, bool in_up_flag)
{
    BaseType::OnKey(in_vk_code, in_scan_code, in_repeat_flag, in_repeat_count, in_up_flag);
    if (_draw_system)
    {
        _draw_system->OnResize();
    }

    if (_resources && _resources->_camera)
    {
        _resources->_camera->OnKey(in_vk_code, in_scan_code, in_repeat_flag, in_repeat_count, in_up_flag);
    }

    return;
}

void Application::OnWindowSizeChanged(const DscCommon::VectorInt2& in_size, const float in_monitor_scale)
{
    BaseType::OnWindowSizeChanged(in_size, in_monitor_scale);
    if (_draw_system)
    {
        _draw_system->OnResize();
    }

    if (_resources && _resources->_camera)
    {
        _resources->_camera->OnResize(in_size);
    }

    return;
}

void Application::StepPosDataIndex()
{
    _pos_data_index = (_pos_data_index + 1) % k_pos_data_count;
    return;
}

const int32 Application::GetPosDataIndex() const
{
    return _pos_data_index;
}

const int32 Application::GetPosDataIndexPrev() const
{
    // +2 == -1, in pos% 3
    return (_pos_data_index + 2) % k_pos_data_count;
}

const int32 Application::GetPosDataIndexPrevPrev() const
{
    // +1 == -2, in pos% 3
    return (_pos_data_index + 1) % k_pos_data_count;
}

void Application::PresentPos(std::unique_ptr<DscRenderResource::Frame>& in_frame, const int32 in_pos_data_index, const DscCommon::VectorFloat4& in_colour)
{
    // present
    //   set T0 (pos)
    {
        DscRenderResource::UnorderedAccess* pos_data = _resources->_pos_data[in_pos_data_index].get();
        in_frame->ResourceBarrier(pos_data, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        _resources->_present_pos_shader->SetShaderResourceViewHandle(0, pos_data->GetShaderViewHeapWrapperItem());
    }
    //   constant buffer
    if (_resources && _resources->_camera)
    {
        auto& buffer = _resources->_present_pos_constant_buffer[in_pos_data_index]->GetConstant<DscCamera::Camera::TConstantBuffer>(0);
        buffer = _resources->_camera->GetConstantBuffer();
    }
    {
        TPresentPosConstantBuffer1& buffer = _resources->_present_pos_constant_buffer[in_pos_data_index]->GetConstant<TPresentPosConstantBuffer1>(1);
        buffer._colour = in_colour;
    }
    //   draw
    in_frame->SetShader(_resources->_present_pos_shader, _resources->_present_pos_constant_buffer[in_pos_data_index]);
    in_frame->Draw(_resources->_geometry);

    return;
}

void Application::UpdateDebugText()
{
    if ((nullptr != _event_store) && (nullptr != _resources) && (nullptr != _resources->_onscreen_debug))
    {
        std::string debug_text;

        _event_store->VisitEvents([&debug_text](const DscStatistics::IEvent& in_event) {
            debug_text += in_event.GetDescription() + "\n";
        });

        _resources->_onscreen_debug->SetText(debug_text);
    }
    return;
}

