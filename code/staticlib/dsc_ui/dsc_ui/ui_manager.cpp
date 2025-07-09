#include "ui_manager.h"

#include "component_construction_helper.h"
#include "make_node.h"
#include "screen_quad.h"
#include "ui_enum.h"
#include "ui_render_target.h"
#include "ui_input_param.h"
#include "ui_input_state.h"
#include <dsc_common\data_helper.h>
#include <dsc_common\file_system.h>
#include <dsc_common\log_system.h>
#include <dsc_common\math.h>
#include <dsc_common\vector_float2.h>
#include <dsc_dag\dag_array_helper.h>
#include <dsc_dag\dag_collection.h>
#include <dsc_dag\dag_group.h>
#include <dsc_dag_render\dag_resource.h>
#include <dsc_render\draw_system.h>
#include <dsc_render\i_render_target.h>
#include <dsc_render_resource\constant_buffer_info.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\geometry_generic.h>
#include <dsc_render_resource\render_target_pool.h>
#include <dsc_render_resource\render_target_texture.h>
#include <dsc_render_resource\shader.h>
#include <dsc_render_resource\shader_constant_buffer.h>
#include <dsc_render_resource\shader_pipeline_state_data.h>
#include <dsc_render_resource\shader_resource.h>
#include <dsc_render_resource\shader_resource_info.h>
#include <dsc_text\text_manager.h>
#include <dsc_text\text_run.h>

namespace
{
    static const std::vector<D3D12_INPUT_ELEMENT_DESC> s_input_element_desc_array({
        D3D12_INPUT_ELEMENT_DESC
        {
            "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
        }
        });

    const DscUi::TUiDrawType GetDrawTypeFromComponentType(const DscUi::TUiComponentType in_component_type)
    {
        switch (in_component_type)
        {
        default:
            DSC_ASSERT_ALWAYS("missing case");
            break;
        case DscUi::TUiComponentType::TDebugGrid:
            return DscUi::TUiDrawType::TDebugGrid;
        case DscUi::TUiComponentType::TFill:
            return DscUi::TUiDrawType::TFill;
        case DscUi::TUiComponentType::TGradientFill:
            return DscUi::TUiDrawType::TGradientFill;
        case DscUi::TUiComponentType::TImage:
            return DscUi::TUiDrawType::TImage;
        case DscUi::TUiComponentType::TCanvas:
            return DscUi::TUiDrawType::TUiPanel;
        case DscUi::TUiComponentType::TText:
            return DscUi::TUiDrawType::TText;
        case DscUi::TUiComponentType::TStack:
            return DscUi::TUiDrawType::TUiPanel;
        case DscUi::TUiComponentType::TButton:
            return DscUi::TUiDrawType::TButton;
        }
        return DscUi::TUiDrawType::TCount;
    }

    const DscUi::TUiDrawType GetDrawTypeFromEffectType(const DscUi::TUiEffectType in_effect_type)
    {
        switch (in_effect_type)
        {
        default:
            DSC_ASSERT_ALWAYS("missing case");
            break;
        case DscUi::TUiEffectType::TEffectCorner:
            return DscUi::TUiDrawType::TEffectCorner;
        case DscUi::TUiEffectType::TEffectDropShadow:
            return DscUi::TUiDrawType::TEffectDropShadow;
        case DscUi::TUiEffectType::TEffectInnerShadow:
            return DscUi::TUiDrawType::TEffectInnerShadow;
        case DscUi::TUiEffectType::TEffectStroke:
            return DscUi::TUiDrawType::TEffectStroke;
        case DscUi::TUiEffectType::TEffectTint:
            return DscUi::TUiDrawType::TEffectTint;
        }
        return DscUi::TUiDrawType::TCount;
    }

    std::shared_ptr<DscRenderResource::Shader> CreateEffectShader(
        DscRender::DrawSystem& in_draw_system,
        DscCommon::FileSystem& in_file_system,
        const std::string& in_vertex_shader_name,
        const std::string& in_pixel_shader_name,
        const bool in_use_data_sampler,
        const int32 in_texture_count = 1
        )
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", in_vertex_shader_name)))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", in_pixel_shader_name)))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            DscUi::ScreenQuad::GetInputElementDesc(),
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                DscUi::TEffectConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
        for (int32 index = 0; index < in_texture_count; ++index)
        {
            array_shader_resource_info.push_back(
                // data sampiler if expecting to be reading source texture at 1:1 scale (no bilinear smear)
                in_use_data_sampler ?
                DscRenderResource::ShaderResourceInfo::FactoryDataSampler(
                    nullptr,
                    D3D12_SHADER_VISIBILITY_PIXEL
                ) : 
                DscRenderResource::ShaderResourceInfo::FactorySampler(
                    nullptr,
                    D3D12_SHADER_VISIBILITY_PIXEL
                )
            );
        }
        return std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info,
            array_shader_constants_info
            );
    }

    void CalculatePanelConstantBuffer(
        DscUi::TUiPanelShaderConstantBuffer& out_buffer,
        const DscCommon::VectorInt2& parent_render_size,
        const DscCommon::VectorInt2& geometry_offset,
        const DscCommon::VectorInt2& geometry_size,
        const DscCommon::VectorFloat2& scroll_value,
        const DscCommon::VectorInt2& render_viewport_size,
        const DscCommon::VectorInt2& render_texture_size
    )
    {
        //float _pos_size[4]; // _pos_x_y_size_width_height;
        // geometry is in range [-1 ... 1], but we want the offset relative to top left
        out_buffer._pos_size[0] = (static_cast<float>(geometry_offset.GetX()) / static_cast<float>(parent_render_size.GetX()) * 2.0f) - 1.0f;
        out_buffer._pos_size[1] = ((1.0f - static_cast<float>(geometry_offset.GetY()) / static_cast<float>(parent_render_size.GetY())) * 2.0f) - 1.0f;
        out_buffer._pos_size[2] = static_cast<float>(geometry_size.GetX()) / static_cast<float>(parent_render_size.GetX()) * 2.0f;
        out_buffer._pos_size[3] = static_cast<float>(geometry_size.GetY()) / static_cast<float>(parent_render_size.GetY()) * 2.0f;

        //float _uv_size[4]; // _ui_x_y_size_width_height;
        const float scroll_x = std::min(1.0f, std::max(0.0f, std::abs(scroll_value.GetX())));
        out_buffer._uv_size[0] = static_cast<float>(render_viewport_size.GetX() - geometry_size.GetX()) * scroll_x / static_cast<float>(render_texture_size.GetX());
        const float scroll_y = std::min(1.0f, std::max(0.0f, std::abs(scroll_value.GetY())));
        out_buffer._uv_size[1] = static_cast<float>(render_viewport_size.GetY() - geometry_size.GetY()) * scroll_y / static_cast<float>(render_texture_size.GetY());
        out_buffer._uv_size[2] = static_cast<float>(geometry_size.GetX()) / static_cast<float>(render_texture_size.GetX());
        out_buffer._uv_size[3] = static_cast<float>(geometry_size.GetY()) / static_cast<float>(render_texture_size.GetY());

        return;
    }

    void TraverseHierarchyInput(
        const DscUi::UiInputParam::TouchData& in_touch,
        const DscUi::ScreenSpace& in_screen_space,
        const DscUi::UiComponentResourceNodeGroup& in_resource_group,
        const std::vector<DscUi::UiNodeGroup>& in_array_children,
        DscUi::UiInputState::TouchState& in_touch_data,
        const bool in_clear_flag,
        bool& in_out_consumed
    )
    {
        const float x = static_cast<float>(in_touch._root_relative_pos.GetX());
        const float y = static_cast<float>(in_touch._root_relative_pos.GetY());

        const bool inside = DscCommon::Math::InsideBounds(x, y, in_screen_space._screen_valid); 
            //&& DscCommon::Math::InsideBounds(x, y, in_screen_space._screen_space);

        DscDag::NodeToken input_state_flag = in_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TInputStateFlag);
        if (nullptr != input_state_flag)
        {
            bool clicked = false;
            DscUi::TUiInputStateFlag flag = inside ? DscUi::TUiInputStateFlag::TRollover : DscUi::TUiInputStateFlag::TNone;
            if ((false == in_out_consumed) && (true == inside))
            {
                if (true == in_touch._active)
                {
                    in_out_consumed = true;
                    if (in_touch_data._click_start)
                    {
                        //flag |= DscUi::TUiInputStateFlag::TClickStart;
                       in_touch_data._node_under_click_start = input_state_flag;
                    }
                }
                else if ((true == in_touch_data._click_end) &&
                    (input_state_flag == in_touch_data._node_under_click_start))
                {
                    in_out_consumed = true;
                    // we have a click
                    //flag |= DscUi::TUiInputStateFlag::TClickEnd;
                    clicked = true;
                }
            }

            // slightly pull this out of the above condition, saves doing it top level of callstack
            if (input_state_flag == in_touch_data._node_under_click_start)
            {
                if (in_touch._active)
                {
                    flag |= DscUi::TUiInputStateFlag::TClick;
                }
                else
                {
                    in_touch_data._node_under_click_start = nullptr;
                }
            }

            if (false == in_clear_flag)
            {
                flag |= DscDag::DagCollection::GetValueType<DscUi::TUiInputStateFlag>(input_state_flag);
                DscDag::DagCollection::SetValueType<DscUi::TUiInputStateFlag>(input_state_flag, flag);
            }
            DscDag::DagCollection::SetValueType<DscUi::TUiInputStateFlag>(input_state_flag, flag);

            if (true == clicked)
            {
                DscDag::NodeToken input_data_node = in_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TInputData);
                if (nullptr != input_data_node)
                {
                    const DscUi::TUiComponentInputData& input_data = DscDag::DagCollection::GetValueType<DscUi::TUiComponentInputData>(input_data_node);
                    if (nullptr != input_data._click_callback)
                    {
                        input_data._click_callback(in_resource_group);
                    }
                }
            }
        }

        for (const auto& child : in_array_children)
        {
            TraverseHierarchyInput(
                in_touch,
                DscDag::DagCollection::GetValueType<DscUi::ScreenSpace>(child.GetNodeToken(DscUi::TUiNodeGroup::TScreenSpace)),
                DscDag::DagCollection::GetValueType<DscUi::UiComponentResourceNodeGroup>(child.GetNodeToken(DscUi::TUiNodeGroup::TUiComponentResources)),
                DscDag::DagCollection::GetValueType<std::vector<DscUi::UiNodeGroup>>(child.GetNodeToken(DscUi::TUiNodeGroup::TArrayChildUiNodeGroup)),
                in_touch_data,
                in_clear_flag,
                in_out_consumed
                );
        }
    }

    void TraverseHierarchyRolloverAccumulate(
        const DscUi::UiComponentResourceNodeGroup& in_resource_group,
        const std::vector<DscUi::UiNodeGroup>& in_array_children,
        const float in_time_delta
        )
    {
        DscDag::NodeToken input_state_flag = in_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TInputStateFlag);
        if (nullptr != input_state_flag)
        {
            DscDag::NodeToken rollover_accumulate = in_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TInputRolloverAccumulate);
            if (nullptr != rollover_accumulate)
            {
                const bool rollover = (0 != (DscDag::DagCollection::GetValueType<DscUi::TUiInputStateFlag>(input_state_flag) & DscUi::TUiInputStateFlag::TRollover));
                float value = DscDag::DagCollection::GetValueType<float>(rollover_accumulate);
                if (true == rollover)
                {
                    value = std::min(1.0f, value + in_time_delta * 3.0f);
                }
                else
                {
                    value = std::max(0.0f, value - in_time_delta * 2.5f);
                }
                DscDag::DagCollection::SetValueType<float>(rollover_accumulate, value);
            }
        }

        for (const auto& child : in_array_children)
        {
            TraverseHierarchyRolloverAccumulate(
                DscDag::DagCollection::GetValueType<DscUi::UiComponentResourceNodeGroup>(child.GetNodeToken(DscUi::TUiNodeGroup::TUiComponentResources)),
                DscDag::DagCollection::GetValueType<std::vector<DscUi::UiNodeGroup>>(child.GetNodeToken(DscUi::TUiNodeGroup::TArrayChildUiNodeGroup)),
                in_time_delta
            );
        }
    }


    void TraverseHierarchyUnlink(
        DscUi::UiComponentResourceNodeGroup& in_resource_group,
        std::vector<DscUi::UiNodeGroup>& in_array_children
        )
    {
        for (auto& child : in_array_children)
        {
            TraverseHierarchyUnlink(
                DscDag::DagCollection::GetValueNonConstRef<DscUi::UiComponentResourceNodeGroup>(child.GetNodeToken(DscUi::TUiNodeGroup::TUiComponentResources), false),
                DscDag::DagCollection::GetValueNonConstRef<std::vector<DscUi::UiNodeGroup>>(child.GetNodeToken(DscUi::TUiNodeGroup::TArrayChildUiNodeGroup), false)
            );
            child.UnlinkOwned();
        }
        in_resource_group.UnlinkOwned();
    }

    void TraverseHierarchyDestroy(
        DscDag::DagCollection& in_dag_collection,
        DscUi::UiComponentResourceNodeGroup& in_resource_group,
        std::vector<DscUi::UiNodeGroup>& in_array_children
        )
    {
        for (auto& child : in_array_children)
        {
            TraverseHierarchyDestroy(
                in_dag_collection,
                DscDag::DagCollection::GetValueNonConstRef<DscUi::UiComponentResourceNodeGroup>(child.GetNodeToken(DscUi::TUiNodeGroup::TUiComponentResources), false),
                DscDag::DagCollection::GetValueNonConstRef<std::vector<DscUi::UiNodeGroup>>(child.GetNodeToken(DscUi::TUiNodeGroup::TArrayChildUiNodeGroup), false)
            );
            child.DeleteOwned(in_dag_collection);
        }
        in_resource_group.DeleteOwned(in_dag_collection);
    }

} // namespace

DscUi::UiManager::UiManager(DscRender::DrawSystem& in_draw_system, DscCommon::FileSystem& in_file_system, DscDag::DagCollection& in_dag_collection)
{
    _dag_resource = DscDagRender::DagResource::Factory(&in_draw_system, &in_dag_collection);
    _render_target_pool = std::make_unique<DscRenderResource::RenderTargetPool>(DscRenderResource::s_default_pixel_alignment);

    //_full_quad_pos_uv
    {
        std::vector<uint8_t> vertex_raw_data;

        //0.0f, 0.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);

        //1.0f, 0.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);

        //0.0f, 1.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);

        //1.0f, 1.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);

        _full_quad_pos_uv = std::make_shared<DscRenderResource::GeometryGeneric>(
            &in_draw_system,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
            ScreenQuad::GetInputElementDesc(),
            vertex_raw_data,
            4
            );
    }

    //_full_quad_pos
    {
        std::vector<uint8_t> vertex_raw_data;

        //0.0f, 0.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);

        //1.0f, 0.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);

        //0.0f, 1.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);

        //1.0f, 1.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, -1.0f);

        _full_quad_pos = std::make_shared<DscRenderResource::GeometryGeneric>(
            &in_draw_system,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
            s_input_element_desc_array,
            vertex_raw_data,
            2
            );
    }

    //_ui_panel_geometry
    {
        std::vector<uint8_t> vertex_raw_data;

        //0.0f, 0.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);

        //0.0f, 1.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);

        //1.0f, 0.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);

        //1.0f, 1.0f,
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 1.0f);
        DscCommon::DataHelper::AppendValue(vertex_raw_data, 0.0f);

        _ui_panel_geometry = std::make_shared<DscRenderResource::GeometryGeneric>(
            &in_draw_system,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
            s_input_element_desc_array,
            vertex_raw_data,
            2
            );
    }

    // _debug_grid_shader
	{
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "debug_grid_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "debug_grid_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to triangle load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            ScreenQuad::GetInputElementDesc(),
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                TDebugGridConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _debug_grid_shader = std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>>(),
            array_shader_constants_info
            );
	}

    // _ui_panel_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "ui_panel_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "ui_panel_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to triangle load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            s_input_element_desc_array,
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),  //CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                TUiPanelShaderConstantBuffer(),
                D3D12_SHADER_VISIBILITY_VERTEX
            )
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
        array_shader_resource_info.push_back(
            // Factory sampler for subpixel accuracy, if we are scrolling, allow possitioning on non pixel boundaries, FactoryDataSampler is goof to force pixel boundiries if needed
            DscRenderResource::ShaderResourceInfo::FactorySampler(
                nullptr,
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _ui_panel_shader = std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info,
            array_shader_constants_info
            );
    }

    // _image_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "image_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "image_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to triangle load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            ScreenQuad::GetInputElementDesc(),
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),  //CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
        array_shader_resource_info.push_back(
            // Factory sampler for subpixel accuracy, may not be 1:1 pixels source to screen size
            DscRenderResource::ShaderResourceInfo::FactorySampler(
                nullptr,
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _image_shader = std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info
            );
    }

    // _fill_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "fill_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "fill_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to triangle load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            s_input_element_desc_array,
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                TFillConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _fill_shader = std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>>(),
            array_shader_constants_info
            );
    }

    //_gradient_fill_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "gradient_fill_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "gradient_fill_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to triangle load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            ScreenQuad::GetInputElementDesc(),
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            DXGI_FORMAT_UNKNOWN,
            render_target_format,
            DscRenderResource::ShaderPipelineStateData::FactoryBlendDescAlphaPremultiplied(),  //CD3DX12_BLEND_DESC(D3D12_DEFAULT),
            CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
            CD3DX12_DEPTH_STENCIL_DESC()// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT)
        );
        std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> array_shader_constants_info;
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                TGradientFillConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _gradient_fill_shader = std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>>(),
            array_shader_constants_info
            );
    }

    _effect_round_corner_shader = CreateEffectShader(
        in_draw_system,
        in_file_system,
        "effect_round_corner_vs.cso",
        "effect_round_corner_ps.cso",
        true
        );

    _effect_drop_shadow_shader = CreateEffectShader(
        in_draw_system,
        in_file_system,
        "effect_drop_shadow_vs.cso",
        "effect_drop_shadow_ps.cso",
        false
    );

    _effect_inner_shadow_shader = CreateEffectShader(
        in_draw_system,
        in_file_system,
        "effect_inner_shadow_vs.cso",
        "effect_inner_shadow_ps.cso",
        false
    );

    _effect_stroke_shader = CreateEffectShader(
        in_draw_system,
        in_file_system,
        "effect_stroke_vs.cso",
        "effect_stroke_ps.cso",
        true
    );

    _effect_tint_shader = CreateEffectShader(
        in_draw_system,
        in_file_system,
        "effect_tint_vs.cso",
        "effect_tint_ps.cso",
        true
    );
}

DscUi::UiManager::~UiManager()
{
    //nop
}

std::shared_ptr<DscUi::UiRenderTarget> DscUi::UiManager::MakeUiRenderTarget(
    DscRender::IRenderTarget* const in_render_target,
    const bool in_allow_clear_on_draw
)
{
    return std::make_shared<DscUi::UiRenderTarget>(in_render_target, in_allow_clear_on_draw);
}

std::shared_ptr<DscUi::UiRenderTarget> DscUi::UiManager::MakeUiRenderTarget(
    const std::shared_ptr<DscRenderResource::RenderTargetTexture>& in_render_target_texture,
    const bool in_allow_clear_on_draw
)
{
    return std::make_shared<DscUi::UiRenderTarget>(in_render_target_texture, in_allow_clear_on_draw);
}

DscUi::UiRootNodeGroup DscUi::UiManager::MakeRootNode(
    const ComponentConstructionHelper& in_construction_helper,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    const std::shared_ptr<UiRenderTarget>& in_ui_render_target,
    const std::vector<TEffectConstructionHelper>& in_effect_array
    )
{
    UiRootNodeGroup result;

    result.SetNodeToken(TUiRootNodeGroup::TFrame, in_dag_collection.CreateValue<DscRenderResource::Frame*>(
        (DscRenderResource::Frame*)(nullptr),
        DscDag::CallbackNever<DscRenderResource::Frame*>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "frame")));

    result.SetNodeToken(TUiRootNodeGroup::TTimeDelta, in_dag_collection.CreateValue<float>(
        0.0f,
        DscDag::CallbackNoZero<float>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "time delta")));

    result.SetNodeToken(TUiRootNodeGroup::TUiScale, in_dag_collection.CreateValue(
        1.0f,
        DscDag::CallbackOnValueChange<float>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "ui scale")));

    result.SetNodeToken(TUiRootNodeGroup::TInputState, in_dag_collection.CreateValue(
        UiInputState(),
        DscDag::CallbackOnSetValue<UiInputState>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "input state")));

    result.SetNodeToken(TUiRootNodeGroup::TUiComponentType, in_dag_collection.CreateValue(
        in_construction_helper._component_type,
        DscDag::CallbackOnValueChange<TUiComponentType>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "ui component type")));

    result.SetNodeToken(TUiRootNodeGroup::TArrayChildUiNodeGroup, in_dag_collection.CreateValue<std::vector<UiNodeGroup>>(
        std::vector<UiNodeGroup>(),
        DscDag::CallbackOnSetValue<std::vector<UiNodeGroup>>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "array child")));

    result.SetNodeToken(TUiRootNodeGroup::TForceDraw, in_dag_collection.CreateValue<bool>(
        false,
        DscDag::CallbackNoZero<bool>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "force draw")));

    result.SetNodeToken(TUiRootNodeGroup::TUiRenderTarget, in_dag_collection.CreateValue(
        in_ui_render_target,
        DscDag::CallbackOnSetValue<std::shared_ptr<DscUi::UiRenderTarget>>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "ui render target")));

    result.SetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize, in_dag_collection.CreateValue(
        DscCommon::VectorInt2::s_zero,
        DscDag::CallbackOnValueChange<DscCommon::VectorInt2>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "render target size")));

    result.SetNodeToken(TUiRootNodeGroup::TScreenSpace, in_dag_collection.CreateValue(
        DscUi::ScreenSpace(),
        DscDag::CallbackOnValueChange<DscUi::ScreenSpace>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "screen space")));

    UiComponentResourceNodeGroup component_resource_node_group;
    component_resource_node_group.SetNodeToken(TUiComponentResourceNodeGroup::TClearColour, in_dag_collection.CreateValue(
        in_construction_helper._clear_colour,
        DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "clear colour")));
    component_resource_node_group.SetNodeToken(TUiComponentResourceNodeGroup::TUiScale, result.GetNodeToken(TUiRootNodeGroup::TUiScale));

    auto draw_node = MakeDrawStack(
        in_construction_helper,
        in_draw_system,
        in_dag_collection,
        in_effect_array,
        result,
        result.GetNodeToken(TUiRootNodeGroup::TUiRenderTarget),
        result.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize),
        result.GetNodeToken(TUiRootNodeGroup::TArrayChildUiNodeGroup),
        component_resource_node_group
        DSC_DEBUG_ONLY(DSC_COMMA "root draw")
    );
    // if force draw is true, we just need to re apply the last draw step, even if nothing else has changed
    DscDag::DagCollection::LinkNodes(result.GetNodeToken(TUiRootNodeGroup::TForceDraw), draw_node);
    DscDag::DagCollection::LinkNodes(result.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize), draw_node);

    result.SetNodeToken(TUiRootNodeGroup::TDrawNode, draw_node);

    component_resource_node_group.Validate();

    result.SetNodeToken(TUiRootNodeGroup::TUiComponentResources, in_dag_collection.CreateValue<DscUi::UiComponentResourceNodeGroup>(
        component_resource_node_group,
        DscDag::CallbackNever<DscUi::UiComponentResourceNodeGroup>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "component resource node group")));

    result.Validate();

    return result;
}

DscUi::UiNodeGroup DscUi::UiManager::ConvertRootNodeGroupToNodeGroup(
    DscDag::DagCollection& in_dag_collection,
    UiRootNodeGroup& in_ui_root_node_group
)
{
    UiNodeGroup result;

    result.SetNodeToken(TUiNodeGroup::TDrawNode, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TDrawNode));
    result.SetNodeToken(TUiNodeGroup::TUiComponentType, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiComponentType));
    result.SetNodeToken(TUiNodeGroup::TUiComponentResources, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiComponentResources));
    result.SetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TArrayChildUiNodeGroup));
    result.SetNodeToken(TUiNodeGroup::TAvaliableSize, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize));
    result.SetNodeToken(TUiNodeGroup::TRenderRequestSize, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize));
    result.SetNodeToken(TUiNodeGroup::TScreenSpace, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TScreenSpace));
    result.SetNodeToken(TUiNodeGroup::TGeometrySize, in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize));
    result.SetNodeToken(TUiNodeGroup::TGeometryOffset, 
        in_dag_collection.CreateValue(
            DscCommon::VectorInt2::s_zero,
            DscDag::CallbackNever<DscCommon::VectorInt2>::Function,
            // use the root and not the result as owner, the result may be thrown away, is just a step to add children to the root...
            &in_ui_root_node_group 
            DSC_DEBUG_ONLY(DSC_COMMA "geometry offset")));
    result.SetNodeToken(TUiNodeGroup::TScrollPos,
        in_dag_collection.CreateValue(
            DscCommon::VectorFloat2::s_zero,
            DscDag::CallbackNever<DscCommon::VectorFloat2>::Function,
            // use the root and not the result as owner, the result may be thrown away, is just a step to add children to the root...
            &in_ui_root_node_group
            DSC_DEBUG_ONLY(DSC_COMMA "scroll pos")));

    result.Validate();

    return result;
}

DscUi::UiNodeGroup DscUi::UiManager::AddChildNode(
    const ComponentConstructionHelper& in_construction_helper,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    const UiRootNodeGroup& in_root_node_group,
    const UiNodeGroup& in_parent,
    const std::vector<TEffectConstructionHelper>& in_effect_array
    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
    )
{
    UiNodeGroup result;

    //TUiComponentResources
    result.SetNodeToken(TUiNodeGroup::TUiComponentResources, in_dag_collection.CreateValue<UiComponentResourceNodeGroup>(
        MakeComponentResourceGroup(
            in_dag_collection, 
            in_construction_helper,
            in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiScale),
            in_parent.GetNodeToken(TUiNodeGroup::TAvaliableSize) // ok, this was ment to be the local avaliable size, but have some dependency order issues here
            ),
        DscDag::CallbackNever<DscUi::UiComponentResourceNodeGroup>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "component resource node group")));

    //TUiComponentType
    result.SetNodeToken(TUiNodeGroup::TUiComponentType, in_dag_collection.CreateValue(
        in_construction_helper._component_type,
        DscDag::CallbackOnValueChange<TUiComponentType>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "ui component type")));

    // TArrayChildUiNodeGroup
    result.SetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup, in_dag_collection.CreateValue(
        std::vector<UiNodeGroup>(),
        DscDag::CallbackOnSetValue<std::vector<UiNodeGroup>>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "array child")));

    //TUiPanelShaderConstantBuffer
    {
        auto panel_shader_constant_buffer = _ui_panel_shader->MakeShaderConstantBuffer(&in_draw_system);
        result.SetNodeToken(TUiNodeGroup::TUiPanelShaderConstantBuffer, in_dag_collection.CreateValue(
            panel_shader_constant_buffer,
            DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
            &result
            DSC_DEBUG_ONLY(DSC_COMMA "ui panel shader constant buffer")));
    }

    //TArrayChildUiNodeGroup
    result.SetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup, in_dag_collection.CreateValue<std::vector<UiNodeGroup>>(
        std::vector<UiNodeGroup>(),
        DscDag::CallbackOnSetValue<std::vector<UiNodeGroup>>::Function,
        &result
        DSC_DEBUG_ONLY(DSC_COMMA "array child")));

    //calculate our avaliable size
    result.SetNodeToken(
        TUiNodeGroup::TAvaliableSize, 
        MakeNode::MakeAvaliableSize(
            in_dag_collection,
            in_parent.GetNodeToken(TUiNodeGroup::TAvaliableSize),
            in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiScale),
            DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
            DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(in_parent.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
            result
        ));

    // more of a order of construction issue than a circular dependency
    if (true == in_construction_helper._has_ui_scale_by_avaliable_width)
    {
        UiComponentResourceNodeGroup& component_resource = DscDag::DagCollection::GetValueNonConstRef<UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources), false);
        DscDag::DagCollection::LinkIndexNodes(1, result.GetNodeToken(TUiNodeGroup::TAvaliableSize), component_resource.GetNodeToken(TUiComponentResourceNodeGroup::TUiScale));
    }

    // calculate our desired size (for stack, this is all the contents, for text, the text render size (if width limit, limit is the avaliable size width))
    DscDag::NodeToken desired_size = MakeNode::MakeDesiredSize(
        in_construction_helper._component_type,
        in_construction_helper._desired_size_from_children_max,
        in_dag_collection,
        in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiScale),
        result.GetNodeToken(TUiNodeGroup::TAvaliableSize),
        result.GetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup),
        DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
        result
        );

    //TGeometrySize
    result.SetNodeToken(TUiNodeGroup::TGeometrySize,
        MakeNode::MakeGeometrySize(
            in_dag_collection,
            DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
            DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(in_parent.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
            desired_size,
            result.GetNodeToken(TUiNodeGroup::TAvaliableSize),
            result
        ));

    //TGeometryOffset (after TGeometrySize as canvas child uses geometry size for attach point)
    result.SetNodeToken(TUiNodeGroup::TGeometryOffset, 
        MakeNode::MakeGeometryOffset(
            in_dag_collection,
            DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
            DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(in_parent.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
            in_parent.GetNodeToken(TUiNodeGroup::TAvaliableSize),
            in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiScale),
            result.GetNodeToken(TUiNodeGroup::TGeometrySize),
            in_parent.GetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup),
            result
        ));
        
    //TRenderRequestSize calculate our render request size (max desired and geometry size)
    result.SetNodeToken(TUiNodeGroup::TRenderRequestSize,
        MakeNode::MakeRenderRequestSize(
            in_dag_collection,
            desired_size,
            result.GetNodeToken(TUiNodeGroup::TGeometrySize),
            result
        ));

    //TScrollPos, // where is the geometry size quad is on the render target texture
    {
        const auto& resource_group = DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources));
        if ((nullptr != resource_group.GetNodeToken(TUiComponentResourceNodeGroup::THasManualScrollX)) &&
            (nullptr != resource_group.GetNodeToken(TUiComponentResourceNodeGroup::THasManualScrollY)) &&
            (nullptr != resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TManualScrollX)) &&
            (nullptr != resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TManualScrollY)))
        {
            DscDag::NodeToken pixel_traversal_node = MakeNode::MakeNodePixelTraversal(
                in_dag_collection,
                result.GetNodeToken(TUiNodeGroup::TGeometrySize),
                result.GetNodeToken(TUiNodeGroup::TRenderRequestSize),
                result
            );

            result.SetNodeToken(TUiNodeGroup::TScrollPos,
                MakeNode::MakeNodeScrollValue(
                    in_dag_collection,
                    DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources)),
                    in_root_node_group.GetNodeToken(TUiRootNodeGroup::TTimeDelta),
                    pixel_traversal_node,
                    result
                ));
        }
        else
        {
            result.SetNodeToken(TUiNodeGroup::TScrollPos, in_dag_collection.CreateValue(
                DscCommon::VectorFloat2(0, 0),
                DscDag::CallbackOnValueChange<DscCommon::VectorFloat2>::Function,
                &result
                DSC_DEBUG_ONLY(DSC_COMMA "scroll pos child")));
        }
    }

    //TScreenSpaceSize, // from top left as 0,0, what is our on screen geometry footprint. for example, this is in mouse space, so if mouse is at [500,400] we want to know if it is inside our screen space to detect rollover
    result.SetNodeToken(TUiNodeGroup::TScreenSpace, MakeNode::MakeScreenSpace(
        in_dag_collection,
        in_parent.GetNodeToken(TUiNodeGroup::TScreenSpace),
        in_parent.GetNodeToken(TUiNodeGroup::TRenderRequestSize),
        result.GetNodeToken(TUiNodeGroup::TGeometrySize),
        result.GetNodeToken(TUiNodeGroup::TGeometryOffset),
        result.GetNodeToken(TUiNodeGroup::TRenderRequestSize),
        result.GetNodeToken(TUiNodeGroup::TScrollPos),
        result
        ));

    UiComponentResourceNodeGroup& component_resource_node_group = DscDag::DagCollection::GetValueNonConstRef<DscUi::UiComponentResourceNodeGroup>(result.GetNodeToken(TUiNodeGroup::TUiComponentResources), false);
    auto draw_node = MakeDrawStack(
        in_construction_helper, //TUiDrawType
        in_draw_system,
        in_dag_collection,
        in_effect_array,
        in_root_node_group,
        nullptr,
        result.GetNodeToken(TUiNodeGroup::TRenderRequestSize),
        result.GetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup),
        component_resource_node_group
        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name + " draw")
    );
    result.SetNodeToken(TUiNodeGroup::TDrawNode, draw_node);

    result.Validate();

    // and now attach to parent
    {
        DscDag::ArrayHelper::PushBack(in_parent.GetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup), result);

        // effect the child Update function to be called just before the parent Update/draw by setting our draw as input of the parent draw
        DscDag::DagCollection::LinkNodes(
            draw_node,
            in_parent.GetNodeToken(TUiNodeGroup::TDrawNode)
            );
    }

    return result;
}


void DscUi::UiManager::DestroyRootNode(
    DscDag::DagCollection& in_dag_collection,
    UiRootNodeGroup& in_root_node_group
    )
{
    TraverseHierarchyUnlink(
        DscDag::DagCollection::GetValueNonConstRef<DscUi::UiComponentResourceNodeGroup>(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiComponentResources), false),
        DscDag::DagCollection::GetValueNonConstRef<std::vector<DscUi::UiNodeGroup>>(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TArrayChildUiNodeGroup), false)
    );
    in_root_node_group.UnlinkOwned();

    TraverseHierarchyDestroy(
        in_dag_collection,
        DscDag::DagCollection::GetValueNonConstRef<DscUi::UiComponentResourceNodeGroup>(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiComponentResources), false),
        DscDag::DagCollection::GetValueNonConstRef<std::vector<DscUi::UiNodeGroup>>(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TArrayChildUiNodeGroup), false)
    );
    in_root_node_group.DeleteOwned(in_dag_collection);

    in_root_node_group = {};
    return;
}

void DscUi::UiManager::RemoveAndDestroyChild(
    DscDag::DagCollection& in_dag_collection,
    const UiNodeGroup& in_parent,
    UiNodeGroup& in_child
)
{
    // unlink the draw nodes
    DscDag::DagCollection::UnlinkNodes(
        in_child.GetNodeToken(TUiNodeGroup::TDrawNode),
        in_parent.GetNodeToken(TUiNodeGroup::TDrawNode)
    );

    // remove the child from the parent child array. slight issue is that we are passing around UiNodeGroups by copy, the nodes they contain may by unique, but the container may be a copy
    std::vector<UiNodeGroup>& parent_array_children = DscDag::DagCollection::GetValueNonConstRef<std::vector<DscUi::UiNodeGroup>>(in_parent.GetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup), false);
    std::vector<UiNodeGroup> temp = {};
    std::swap(parent_array_children, temp); // does this work?
    auto child_draw_node = in_child.GetNodeToken(DscUi::TUiNodeGroup::TDrawNode);
    for (auto& item: temp)
    {
        if (child_draw_node != item.GetNodeToken(DscUi::TUiNodeGroup::TDrawNode))
        {
            parent_array_children.push_back(item);
        }
    }

    TraverseHierarchyUnlink(
        DscDag::DagCollection::GetValueNonConstRef<DscUi::UiComponentResourceNodeGroup>(in_child.GetNodeToken(TUiNodeGroup::TUiComponentResources), false),
        DscDag::DagCollection::GetValueNonConstRef<std::vector<DscUi::UiNodeGroup>>(in_child.GetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup), false)
    );
    in_child.UnlinkOwned();

    TraverseHierarchyDestroy(
        in_dag_collection,
        DscDag::DagCollection::GetValueNonConstRef<DscUi::UiComponentResourceNodeGroup>(in_child.GetNodeToken(TUiNodeGroup::TUiComponentResources), false),
        DscDag::DagCollection::GetValueNonConstRef<std::vector<DscUi::UiNodeGroup>>(in_child.GetNodeToken(TUiNodeGroup::TArrayChildUiNodeGroup), false)
    );
    in_child.DeleteOwned(in_dag_collection);

    return;
}

void DscUi::UiManager::Update(
    const UiRootNodeGroup& in_root_node_group,
    const float in_time_delta,
    const UiInputParam& in_input_param,
    DscRender::IRenderTarget* const in_external_render_target_or_null
)
{
    DscDag::DagCollection::SetValueType(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TTimeDelta), in_time_delta);

    if (in_external_render_target_or_null)
    {
        DscDag::NodeToken node = in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiRenderTarget);
        auto render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(node);
        DSC_ASSERT(nullptr != render_target, "invalid state");
        render_target->UpdateExternalRenderTarget(in_external_render_target_or_null);
    }
    UpdateRootViewportSize(in_root_node_group);

    UiInputState& input_state = DscDag::DagCollection::GetValueNonConstRef<UiInputState>(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TInputState), false);

    //travers node hierarcy with the in_input_state updating a UiInputInternal to effect state/ button clicks/ rollover
    //multiple touches may also be the keyboard navigation? 
    //if there is only going to be one touch, then combine TraverseHierarchyInput and TraverseHierarchyRolloverAccumulate
    bool first = true;
    for (const auto& touch : in_input_param._touch_data_array)
    {
        bool consumed = false;
        TraverseHierarchyInput(
            touch,
            DscDag::DagCollection::GetValueType<DscUi::ScreenSpace>(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TScreenSpace)),
            DscDag::DagCollection::GetValueType<DscUi::UiComponentResourceNodeGroup>(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiComponentResources)),
            DscDag::DagCollection::GetValueType<std::vector<DscUi::UiNodeGroup>>(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TArrayChildUiNodeGroup)),
            input_state.GetTouchState(touch),
            first,
            consumed
            );
        first = false;
    }

    TraverseHierarchyRolloverAccumulate(
        DscDag::DagCollection::GetValueType<DscUi::UiComponentResourceNodeGroup>(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiComponentResources)),
        DscDag::DagCollection::GetValueType<std::vector<DscUi::UiNodeGroup>>(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TArrayChildUiNodeGroup)),
        in_time_delta
    );

    return;
}

void DscUi::UiManager::Draw(
    const UiRootNodeGroup& in_root_node_group,
    DscDag::DagCollection& in_dag_collection,
    DscRenderResource::Frame& in_frame,
    const bool in_force_draw,
    DscRender::IRenderTarget* const in_external_render_target_or_null
)
{
    DscDag::DagCollection::SetValueType(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TFrame), &in_frame);
    DscDag::DagCollection::SetValueType(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TForceDraw), in_force_draw);

    if (in_external_render_target_or_null)
    {
        DscDag::NodeToken node = in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiRenderTarget);
        auto render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(node);
        DSC_ASSERT(nullptr != render_target, "invalid state");
        render_target->UpdateExternalRenderTarget(in_external_render_target_or_null);
    }
    UpdateRootViewportSize(in_root_node_group);

    in_dag_collection.ResolveDirtyConditionNodes();

    in_root_node_group.GetNodeToken(TUiRootNodeGroup::TDrawNode)->Update();

    return;
}

void DscUi::UiManager::UpdateRootViewportSize(
    const UiRootNodeGroup& in_root_node_group
    )
{
    DscDag::NodeToken node = in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiRenderTarget);
    auto render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(node);
    DSC_ASSERT(nullptr != render_target, "invalid state");

    if (nullptr != render_target)
    {
        const DscCommon::VectorInt2 viewport_size = render_target->GetViewportSize();
        DscDag::DagCollection::SetValueType(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize), viewport_size);

        DscUi::ScreenSpace screen_space({ DscCommon::VectorFloat4(
            0.0f,
            0.0f,
            static_cast<float>(viewport_size.GetX()),
            static_cast<float>(viewport_size.GetY())
        ), DscCommon::VectorFloat4(
            0.0f,
            0.0f,
            static_cast<float>(viewport_size.GetX()),
            static_cast<float>(viewport_size.GetY())
        ) });

        DscDag::DagCollection::SetValueType(in_root_node_group.GetNodeToken(TUiRootNodeGroup::TScreenSpace), screen_space);
    }
    return;
}

DscDag::NodeToken DscUi::UiManager::MakeDrawStack(
    const ComponentConstructionHelper& in_construction_helper,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    const std::vector<TEffectConstructionHelper>& in_effect_array,
    const UiRootNodeGroup& in_root_node_group,
    DscDag::NodeToken in_last_render_target_or_null,
    DscDag::NodeToken in_render_request_size_node,
    DscDag::NodeToken in_child_array_node_or_null,
    UiComponentResourceNodeGroup& in_component_resource_group
    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
)
{
    std::vector<DscDag::NodeToken> array_draw_nodes;
    std::vector<DscDag::NodeToken> effect_param_array;
    DscDag::NodeToken last_draw_node = nullptr;

    {
        DscDag::NodeToken ui_render_target_node = nullptr;
        if ((0 == in_effect_array.size()) && (nullptr != in_last_render_target_or_null))
        {
            ui_render_target_node = in_last_render_target_or_null;
        }
        else
        {
            DscDag::NodeToken component_clear_colour = in_component_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TClearColour);

            ui_render_target_node = MakeNode::MakeUiRenderTargetNode(
                in_draw_system, 
                *_render_target_pool, 
                in_dag_collection, 
                component_clear_colour,
                in_render_request_size_node,
                in_component_resource_group
                );
        }

        last_draw_node = MakeDrawNode(
            GetDrawTypeFromComponentType(in_construction_helper._component_type),
            &in_construction_helper,
            in_draw_system,
            in_dag_collection,
            array_draw_nodes,
            in_root_node_group.GetNodeToken(TUiRootNodeGroup::TFrame),
            ui_render_target_node,
            in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiScale),
            nullptr,
            nullptr,
            in_child_array_node_or_null,
            in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
        );
        array_draw_nodes.push_back(last_draw_node);
    }
    if (0 < in_effect_array.size())
    {
        std::vector<DscDag::NodeToken> array_effect_param_nodes;

        for (size_t index = 0; index < in_effect_array.size(); ++index)
        {
            DscDag::NodeToken ui_render_target_node = nullptr;
            if ((index == in_effect_array.size() - 1) && (nullptr != in_last_render_target_or_null))
            {
                ui_render_target_node = in_last_render_target_or_null;
            }
            else
            {
                DscDag::NodeToken effect_clear_colour = in_dag_collection.CreateValue(
                    DscCommon::VectorFloat4::s_zero,
                    DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
                    &in_component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "effect clear colour"));

                // you could put this in the effect_param_array, but then the stride of the effect param could be weird
                // could also put it in in_component_resource_group? have it's own array of 

                ui_render_target_node = MakeNode::MakeUiRenderTargetNode(
                    in_draw_system, 
                    *_render_target_pool, 
                    in_dag_collection, 
                    effect_clear_colour, 
                    in_render_request_size_node,
                    in_component_resource_group
                    );
            }

            const auto& effect_data = in_effect_array[index];
            DscDag::NodeToken effect_param = nullptr;
            DscDag::NodeToken effect_tint = nullptr;
            if (true == effect_data._use_rollover_param_lerp)
            {
                DSC_ASSERT(nullptr != in_component_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TInputRolloverAccumulate), "invalid state");

                DscDag::NodeToken effect_param_0 = in_dag_collection.CreateValue(
                    effect_data._effect_param,
                    DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
                    &in_component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "effect param 0"));
                effect_param_array.push_back(effect_param_0);
                DscDag::NodeToken effect_tint_0 = in_dag_collection.CreateValue(
                    effect_data._effect_param_tint,
                    DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
                    &in_component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "effect tint 0"));
                effect_param_array.push_back(effect_tint_0);
                DscDag::NodeToken effect_param_1 = in_dag_collection.CreateValue(
                    effect_data._effect_param_rollover,
                    DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
                    &in_component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "effect param 1"));
                effect_param_array.push_back(effect_param_1);
                DscDag::NodeToken effect_tint_1 = in_dag_collection.CreateValue(
                    effect_data._effect_param_tint_rollover,
                    DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
                    &in_component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "effect tint 1"));
                effect_param_array.push_back(effect_tint_1);

                effect_param = MakeNode::MakeLerpFloat4(
                    in_dag_collection,
                    in_component_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TInputRolloverAccumulate),
                    effect_param_0,
                    effect_param_1,
                    in_component_resource_group
                    );

                effect_tint = MakeNode::MakeLerpFloat4(
                    in_dag_collection,
                    in_component_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TInputRolloverAccumulate),
                    effect_tint_0,
                    effect_tint_1,
                    in_component_resource_group
                );

            }
            else
            {
                effect_param = in_dag_collection.CreateValue(
                    effect_data._effect_param,
                    DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
                    &in_component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "effect param"));
                effect_param_array.push_back(effect_param);
                effect_tint = in_dag_collection.CreateValue(
                    effect_data._effect_param_tint,
                    DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
                    &in_component_resource_group
                    DSC_DEBUG_ONLY(DSC_COMMA "effect tint"));
                effect_param_array.push_back(effect_tint);
            }

            last_draw_node = MakeDrawNode(
                GetDrawTypeFromEffectType(effect_data._effect_type),
                nullptr,
                in_draw_system,
                in_dag_collection,
                array_draw_nodes,
                in_root_node_group.GetNodeToken(TUiRootNodeGroup::TFrame),
                ui_render_target_node,
                in_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiScale),
                effect_param,
                effect_tint,
                in_child_array_node_or_null,
                in_component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "effect draw")
            );
            array_draw_nodes.push_back(last_draw_node);
        }
    }

    if (0 < effect_param_array.size())
    {
        in_component_resource_group.SetNodeToken(TUiComponentResourceNodeGroup::TEffectParamArray,
            in_dag_collection.CreateValue(
                effect_param_array,
                DscDag::CallbackNever<std::vector<DscDag::NodeToken>>::Function,
                &in_component_resource_group
                DSC_DEBUG_ONLY(DSC_COMMA "effect param array")
            ));
    }

    DSC_ASSERT(nullptr != last_draw_node, "invalid state");

    return last_draw_node;
}

DscDag::NodeToken DscUi::UiManager::MakeDrawNode(
    const TUiDrawType in_type,
    const ComponentConstructionHelper* const in_construction_helper_or_null,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    std::vector<DscDag::NodeToken>& in_array_input_stack,
    DscDag::NodeToken in_frame_node,
    DscDag::NodeToken in_ui_render_target_node,
    DscDag::NodeToken in_ui_scale,
    DscDag::NodeToken in_effect_param_or_null,
    DscDag::NodeToken in_effect_tint_or_null,
    DscDag::NodeToken in_child_array_node_or_null,
    UiComponentResourceNodeGroup& in_component_resource_group
    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
)
{
    DSC_ASSERT(nullptr != in_frame_node, "invalid param");
    //DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "MakeDrawNode in_frame_node:%s TYPE:%s\n", in_frame_node->GetTypeInfo().name(), typeid(DscRenderResource::Frame*).name());
    DSC_ASSERT(in_frame_node->GetTypeInfo() == typeid(DscRenderResource::Frame*), "invalid param");

    DSC_ASSERT(nullptr != in_ui_scale, "invalid param");
    //DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "MakeDrawNode in_ui_scale:%s TYPE:%s\n", in_ui_scale->GetTypeInfo().name(), typeid(float).name());
    DSC_ASSERT(in_ui_scale->GetTypeInfo() == typeid(float), "invalid param");

    DscDag::NodeToken result_node = {};
    switch (in_type)
    {
    default:
        DSC_ASSERT_ALWAYS("missing switch");
        break;
    case TUiDrawType::TDebugGrid:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _full_quad_pos_uv;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _debug_grid_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            auto shader_buffer = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[2]);
            DSC_ASSERT(nullptr != shader_buffer, "invalid state");

            const DscCommon::VectorInt2 viewport_size = ui_render_target->GetViewportSize();

            auto& buffer = shader_buffer->GetConstant<TDebugGridConstantBuffer>(0);
            buffer._width_height[0] = static_cast<float>(viewport_size.GetX());
            buffer._width_height[1] = static_cast<float>(viewport_size.GetY());

            if (true == ui_render_target->ActivateRenderTarget(*frame))
            {
                frame->SetShader(weak_shader.lock(), shader_buffer);
                frame->Draw(weak_geometry.lock());
                frame->SetRenderTarget(nullptr);
            }

            out_value = ui_render_target.get();
        },
            & in_component_resource_group
        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));

        auto shader_buffer = _debug_grid_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto shader_buffer_node = in_dag_collection.CreateValue(
            shader_buffer,
            DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
            &in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "shader constant"));

        DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(2, shader_buffer_node, result_node);
    }
    break;
    case TUiDrawType::TFill:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _full_quad_pos;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _fill_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            auto shader_buffer = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[2]);
            DSC_ASSERT(nullptr != shader_buffer, "invalid state");
            const DscCommon::VectorFloat4& fill_colour = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat4>(in_input_array[3]);

            auto& buffer = shader_buffer->GetConstant<TFillConstantBuffer>(0);
            buffer._colour[0] = fill_colour.GetX();
            buffer._colour[1] = fill_colour.GetY();
            buffer._colour[2] = fill_colour.GetZ();
            buffer._colour[3] = fill_colour.GetW();

            if (true == ui_render_target->ActivateRenderTarget(*frame))
            {
                frame->SetShader(weak_shader.lock(), shader_buffer);
                frame->Draw(weak_geometry.lock());
                frame->SetRenderTarget(nullptr);
            }

            out_value = ui_render_target.get();
        },
            & in_component_resource_group
        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));

        auto shader_buffer = _fill_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto shader_buffer_node = in_dag_collection.CreateValue(
            shader_buffer,
            DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
            &in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "shader constant"));

        DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(2, shader_buffer_node, result_node);
        DSC_ASSERT(nullptr != in_component_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TFillColour), "invalid state");
        DscDag::DagCollection::LinkIndexNodes(3, in_component_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TFillColour), result_node);
    }
    break;
    case TUiDrawType::TGradientFill:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _full_quad_pos_uv;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _gradient_fill_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            auto shader_buffer = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[2]);
            DSC_ASSERT(nullptr != shader_buffer, "invalid state");

            const TGradientFillConstantBuffer& gradient_fill = DscDag::DagCollection::GetValueType<TGradientFillConstantBuffer>(in_input_array[3]);
            shader_buffer->GetConstant<TGradientFillConstantBuffer>(0) = gradient_fill;

            if (true == ui_render_target->ActivateRenderTarget(*frame))
            {
                frame->SetShader(weak_shader.lock(), shader_buffer);
                frame->Draw(weak_geometry.lock());
                frame->SetRenderTarget(nullptr);
            }

            out_value = ui_render_target.get();
        },
            &in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));

        auto shader_buffer = _gradient_fill_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto shader_buffer_node = in_dag_collection.CreateValue(
            shader_buffer,
            DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
            &in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "shader constant"));

        DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(2, shader_buffer_node, result_node);
        DSC_ASSERT(nullptr != in_component_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TGradientFill), "invalid state");
        DscDag::DagCollection::LinkIndexNodes(3, in_component_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TGradientFill), result_node);
    }
    break;
    case TUiDrawType::TImage:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _full_quad_pos_uv;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _image_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            auto shader_resource = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::ShaderResource>>(in_input_array[2]);

            if (true == ui_render_target->ActivateRenderTarget(*frame))
            {
                auto shader = weak_shader.lock();
                shader->SetShaderResourceViewHandle(0, shader_resource->GetHeapWrapperItem());
                frame->SetShader(shader);
                frame->Draw(weak_geometry.lock());
                frame->SetRenderTarget(nullptr);
            }

            out_value = ui_render_target.get();
        },
            & in_component_resource_group
        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));

        DSC_ASSERT(nullptr != in_construction_helper_or_null, "invalid state");
        auto texture_node = in_dag_collection.CreateValue(
            in_construction_helper_or_null->_texture,
            DscDag::CallbackOnSetValue<std::shared_ptr<DscRenderResource::ShaderResource>>::Function,
            &in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA "fill colour"));
        in_component_resource_group.SetNodeToken(TUiComponentResourceNodeGroup::TTexture, texture_node);

        DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(2, texture_node, result_node);
    }
    break;
    case TUiDrawType::TUiPanel:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _ui_panel_geometry;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _ui_panel_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            const auto& ui_render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            const std::vector<UiNodeGroup>& child_array = DscDag::DagCollection::GetValueType<std::vector<UiNodeGroup>>(in_input_array[2]);

            if (true == ui_render_target->ActivateRenderTarget(*frame))
            {
                auto shader = weak_shader.lock();
                auto geometry = weak_geometry.lock();
                const DscCommon::VectorInt2 parent_render_size = ui_render_target->GetViewportSize();

                for (const auto& child : child_array)
                {
                    DscUi::UiRenderTarget* child_render_target = DscDag::DagCollection::GetValueType<DscUi::UiRenderTarget*>(child.GetNodeToken(TUiNodeGroup::TDrawNode));
                    DSC_ASSERT(nullptr != child_render_target, "invalid state");
                    auto child_texture = child_render_target->GetTexture();
                    if (nullptr == child_texture)
                    {
                        continue;
                    }

                    const auto& shader_constant_buffer = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(child.GetNodeToken(TUiNodeGroup::TUiPanelShaderConstantBuffer));

                    auto& buffer = shader_constant_buffer->GetConstant<TUiPanelShaderConstantBuffer>(0);
                    const DscCommon::VectorInt2& geometry_offset = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(child.GetNodeToken(TUiNodeGroup::TGeometryOffset));
                    const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(child.GetNodeToken(TUiNodeGroup::TGeometrySize));
                    const DscCommon::VectorFloat2& scroll_value = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat2>(child.GetNodeToken(TUiNodeGroup::TScrollPos));
                    const DscCommon::VectorInt2 render_viewport_size = child_render_target->GetViewportSize();
                    const DscCommon::VectorInt2 render_texture_size = child_render_target->GetTextureSize();

                    CalculatePanelConstantBuffer(
                        buffer,
                        parent_render_size,
                        geometry_offset,
                        geometry_size,
                        scroll_value,
                        render_viewport_size,
                        render_texture_size
                        );

                    shader->SetShaderResourceViewHandle(0, child_texture);
                    frame->SetShader(shader, shader_constant_buffer);
                    frame->Draw(geometry);
                }
            }

            out_value = ui_render_target.get();
        },
            & in_component_resource_group
        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));

        DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DSC_ASSERT(nullptr != in_child_array_node_or_null, "invalid state");
        DscDag::DagCollection::LinkIndexNodes(2, in_child_array_node_or_null, result_node);
    }
    break;
    case TUiDrawType::TButton: // like TUiPanel, but we filter the draw of the children if they have a TForInputStateFlag not matching the currents nodes TInputStateFlag
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _ui_panel_geometry;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _ui_panel_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            const auto& ui_render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            const std::vector<UiNodeGroup>& child_array = DscDag::DagCollection::GetValueType<std::vector<UiNodeGroup>>(in_input_array[2]);
            const TUiInputStateFlag input_state_flag = DscDag::DagCollection::GetValueType<TUiInputStateFlag>(in_input_array[3]);

            if (true == ui_render_target->ActivateRenderTarget(*frame))
            {
                auto shader = weak_shader.lock();
                auto geometry = weak_geometry.lock();
                const DscCommon::VectorInt2 parent_render_size = ui_render_target->GetViewportSize();

                for (const auto& child : child_array)
                {
                    //bail if the child has a non matching input_state_flag, if it has one
                    const UiComponentResourceNodeGroup& child_resource_group = DscDag::DagCollection::GetValueType<UiComponentResourceNodeGroup>(child.GetNodeToken(TUiNodeGroup::TUiComponentResources));
                    DscDag::NodeToken for_input_state_node = child_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TForInputStateFlag);
                    if (nullptr != for_input_state_node)
                    {
                        const TUiInputStateFlag child_input_state_flag = DscDag::DagCollection::GetValueType<TUiInputStateFlag>(for_input_state_node);
                        if (input_state_flag != child_input_state_flag)
                        {
                            continue;
                        }
                    }

                    DscUi::UiRenderTarget* child_render_target = DscDag::DagCollection::GetValueType<DscUi::UiRenderTarget*>(child.GetNodeToken(TUiNodeGroup::TDrawNode));
                    DSC_ASSERT(nullptr != child_render_target, "invalid state");
                    auto child_texture = child_render_target->GetTexture();
                    if (nullptr == child_texture)
                    {
                        continue;
                    }

                    const auto& shader_constant_buffer = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(child.GetNodeToken(TUiNodeGroup::TUiPanelShaderConstantBuffer));

                    auto& buffer = shader_constant_buffer->GetConstant<TUiPanelShaderConstantBuffer>(0);
                    const DscCommon::VectorInt2& geometry_offset = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(child.GetNodeToken(TUiNodeGroup::TGeometryOffset));
                    const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(child.GetNodeToken(TUiNodeGroup::TGeometrySize));
                    const DscCommon::VectorFloat2& scroll_value = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat2>(child.GetNodeToken(TUiNodeGroup::TScrollPos));
                    const DscCommon::VectorInt2 render_viewport_size = child_render_target->GetViewportSize();
                    const DscCommon::VectorInt2 render_texture_size = child_render_target->GetTextureSize();

                    CalculatePanelConstantBuffer(
                        buffer,
                        parent_render_size,
                        geometry_offset,
                        geometry_size,
                        scroll_value,
                        render_viewport_size,
                        render_texture_size
                    );

                    shader->SetShaderResourceViewHandle(0, child_texture);
                    frame->SetShader(shader, shader_constant_buffer);
                    frame->Draw(geometry);
                }
            }

            out_value = ui_render_target.get();
        },
            &in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));

        DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DSC_ASSERT(nullptr != in_child_array_node_or_null, "invalid state");
        DscDag::DagCollection::LinkIndexNodes(2, in_child_array_node_or_null, result_node);
        DscDag::DagCollection::LinkIndexNodes(3, in_component_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TInputStateFlag), result_node);
    }
    break;
    case TUiDrawType::TText:
    {
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                DscRenderResource::Frame& frame = *DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
                //DSC_ASSERT(nullptr != frame, "invalid state");
                const auto& ui_render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
                DSC_ASSERT(nullptr != ui_render_target, "invalid state");
                const TUiComponentTextData& text_data = DscDag::DagCollection::GetValueType<TUiComponentTextData>(in_input_array[2]);

                if (true == ui_render_target->ActivateRenderTarget(frame))
                {
                    DscText::TextRun* text_run_raw = text_data._text_run.get();
                    if (nullptr != text_run_raw)
                    {
                        text_run_raw->SetTextContainerSize(ui_render_target->GetViewportSize());

                        auto geometry = text_run_raw->GetGeometry(&frame.GetDrawSystem(), &frame);
                        DSC_ASSERT(nullptr != text_data._text_manager, "invalid state");
                        auto shader = text_data._text_manager->GetShader(&frame.GetDrawSystem(), &frame);
                        frame.SetShader(shader);
                        frame.Draw(geometry);
                    }
                }

                out_value = ui_render_target.get();
            },
            &in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));

        DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::DagCollection::LinkIndexNodes(2, in_component_resource_group.GetNodeToken(TUiComponentResourceNodeGroup::TText), result_node);

    }
    break;
    case TUiDrawType::TEffectCorner:
        result_node = MakeNode::MakeEffectDrawNode(
            _full_quad_pos_uv,
            _effect_round_corner_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
    break;
    case TUiDrawType::TEffectDropShadow:
        result_node = MakeNode::MakeEffectDrawNode(
            _full_quad_pos_uv,
            _effect_drop_shadow_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
        break;
    case TUiDrawType::TEffectInnerShadow:
        result_node = MakeNode::MakeEffectDrawNode(
            _full_quad_pos_uv,
            _effect_inner_shadow_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
        break;
    case TUiDrawType::TEffectStroke:
        result_node = MakeNode::MakeEffectDrawNode(
            _full_quad_pos_uv,
            _effect_stroke_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
        break;
    case TUiDrawType::TEffectTint:
        result_node = MakeNode::MakeEffectDrawNode(
            _full_quad_pos_uv,
            _effect_tint_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            in_component_resource_group
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
        break;

    }
    return result_node;
}
