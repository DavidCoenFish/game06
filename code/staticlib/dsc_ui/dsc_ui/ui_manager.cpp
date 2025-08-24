#include "ui_manager.h"

#include "celtic_knot.h"
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
#include <dsc_dag\dag_collection.h>
#include <dsc_dag\dag_node_group.h>
#include <dsc_dag\debug_print.h>
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

bool k_rollover = false;

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
        case DscUi::TUiComponentType::TCelticKnotFill:
            return DscUi::TUiDrawType::TCelticKnotFill;
        case DscUi::TUiComponentType::TImage:
            return DscUi::TUiDrawType::TImage;
        case DscUi::TUiComponentType::TCanvas:
            return DscUi::TUiDrawType::TUiPanel;
        case DscUi::TUiComponentType::TText:
            return DscUi::TUiDrawType::TText;
        case DscUi::TUiComponentType::TTextNode:
            return DscUi::TUiDrawType::TText;
        case DscUi::TUiComponentType::TStack:
            return DscUi::TUiDrawType::TUiPanel;
        case DscUi::TUiComponentType::TCrossFade:
            return DscUi::TUiDrawType::TUiPanel;
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
        case DscUi::TUiEffectType::TEffectBlur:
            return DscUi::TUiDrawType::TEffectBlur;
        case DscUi::TUiEffectType::TEffectDesaturate:
            return DscUi::TUiDrawType::TEffectDesaturate;
        case DscUi::TUiEffectType::TEffectBurnBlot:
            return DscUi::TUiDrawType::TEffectBurnBlot;
        case DscUi::TUiEffectType::TEffectBurnPresent:
            return DscUi::TUiDrawType::TEffectBurnPresent;
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
        DscUi::TUiPanelShaderConstantBufferVS& out_buffer,
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
        DscDag::NodeToken in_ui_node_group,
        DscUi::UiInputState::TouchState& in_touch_data,
        const bool in_clear_flag,
        bool& in_out_consumed,
        bool& in_out_ignore
    )
    {
        const float x = static_cast<float>(in_touch._root_relative_pos.GetX());
        const float y = static_cast<float>(in_touch._root_relative_pos.GetY());

        const DscUi::ScreenSpace& screen_space = DscDag::GetValueType<DscUi::ScreenSpace>(
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_ui_node_group, DscUi::TUiNodeGroup::TScreenSpace)
            );

        bool inside = DscCommon::Math::InsideBounds(x, y, screen_space._screen_valid); 
        if (true == in_out_ignore)
        {
            inside = false;
        }

        DscDag::NodeToken resource_group = DscDag::DagNodeGroup::GetNodeTokenEnum(in_ui_node_group, DscUi::TUiNodeGroup::TUiComponentResources);
        DscDag::NodeToken input_flow_amount_node = DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, DscUi::TUiComponentResourceNodeGroup::TInputFlowBehaviour);
        const DscUi::TUiInputFlowBehaviour input_flow_behaviour = (nullptr != input_flow_amount_node) ?
            DscDag::GetValueType<DscUi::TUiInputFlowBehaviour>(input_flow_amount_node) : DscUi::TUiInputFlowBehaviour::TNormal;

        if (DscUi::TUiInputFlowBehaviour::TIgnore == input_flow_behaviour)
        {
            inside = false;
        }

        DscDag::NodeToken input_state_flag = DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, DscUi::TUiComponentResourceNodeGroup::TInputStateFlag);
        if (nullptr != input_state_flag)
        {
            k_rollover = inside;
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
                flag |= DscDag::GetValueType<DscUi::TUiInputStateFlag>(input_state_flag);
            }

            DscDag::NodeToken selected_item_selected_node = DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, DscUi::TUiComponentResourceNodeGroup::TSelectedItemSelected);
            if (nullptr != selected_item_selected_node)
            {
                const bool is_selected = DscDag::GetValueType<bool>(selected_item_selected_node);
                if (true == is_selected)
                {
                    flag |= DscUi::TUiInputStateFlag::TSelection;
                }
                else
                {
                    flag &= ~DscUi::TUiInputStateFlag::TSelection;
                }
            }

            DscDag::SetValueType<DscUi::TUiInputStateFlag>(input_state_flag, flag);

            if (true == clicked)
            {
                DscDag::NodeToken input_data_node = DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, DscUi::TUiComponentResourceNodeGroup::TInputData);
                if (nullptr != input_data_node)
                {
                    const DscUi::TUiComponentInputData& input_data = DscDag::GetValueType<DscUi::TUiComponentInputData>(input_data_node);
                    if (nullptr != input_data._click_callback)
                    {
                        input_data._click_callback(in_ui_node_group);
                    }
                }
            }

            DscDag::NodeToken active_touch_pos = DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, DscUi::TUiComponentResourceNodeGroup::TInputActiveTouchPos);
            if (nullptr != active_touch_pos)
            {
                //screen_space, origin top left?
                //in_touch_data origin, top left
                const DscCommon::VectorFloat2 relative_mouse_pos(
                    x - screen_space._screen_space[0],
                    y - screen_space._screen_space[1]
                    );
                DscDag::SetValueType(active_touch_pos, relative_mouse_pos);
            }
        }

        DscDag::NodeToken array_children_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_ui_node_group, DscUi::TUiNodeGroup::TArrayChildUiNodeGroup);
        const auto& array_children = DscDag::GetValueNodeArray(array_children_node);
        for (auto iter = array_children.rbegin(); iter != array_children.rend(); ++iter)
        {
            bool dummy_local = true;
            bool& ignore_local = (DscUi::TUiInputFlowBehaviour::TIgnore == input_flow_behaviour) ?
                dummy_local : in_out_ignore;
            DscDag::NodeToken child = *iter;
            TraverseHierarchyInput(
                in_touch,
                child,
                in_touch_data,
                in_clear_flag,
                in_out_consumed,
                ignore_local
                );
        }

        if (DscUi::TUiInputFlowBehaviour::TModal == input_flow_behaviour)
        {
            in_out_ignore = true;
        }
    }

    void TraverseHierarchyRolloverAccumulate(
        DscDag::NodeToken in_ui_node_group,
        const float in_time_delta
        )
    {
        DscDag::NodeToken resource_group = DscDag::DagNodeGroup::GetNodeTokenEnum(in_ui_node_group, DscUi::TUiNodeGroup::TUiComponentResources);
        DscDag::NodeToken input_state_flag = DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, DscUi::TUiComponentResourceNodeGroup::TInputStateFlag);

        if (nullptr != input_state_flag)
        {
            DscDag::NodeToken rollover_accumulate = DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, DscUi::TUiComponentResourceNodeGroup::TInputRolloverAccumulate);
            if (nullptr != rollover_accumulate)
            {
                const bool rollover = (0 != (DscDag::GetValueType<DscUi::TUiInputStateFlag>(input_state_flag) & DscUi::TUiInputStateFlag::TRollover));
                float value = DscDag::GetValueType<float>(rollover_accumulate);
                if (true == rollover)
                {
                    value = std::min(1.0f, value + in_time_delta * 3.0f);
                }
                else
                {
                    value = std::max(0.0f, value - in_time_delta * 2.5f);
                }
                DscDag::SetValueType<float>(rollover_accumulate, value);
            }
        }

        DscDag::NodeToken array_children_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_ui_node_group, DscUi::TUiNodeGroup::TArrayChildUiNodeGroup);
        const auto& array_children = DscDag::GetValueNodeArray(array_children_node);
        for (const auto& child : array_children)
        {
            TraverseHierarchyRolloverAccumulate(
                child,
                in_time_delta
            );
        }
    }

    void TraverseHierarchyUnlink(
        DscDag::NodeToken in_ui_node_group
        )
    {
        DscDag::NodeToken array_children_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_ui_node_group, DscUi::TUiNodeGroup::TArrayChildUiNodeGroup);
        const auto& array_children = DscDag::GetValueNodeArray(array_children_node);
        for (auto& child : array_children)
        {
            TraverseHierarchyUnlink(
                child
            );
        }
        in_ui_node_group->UnlinkInputs();
    }


    void TraverseHierarchyDeleteNode(
        DscDag::NodeToken in_ui_node_group,
        DscDag::DagCollection& in_dag_collection
    )
    {
        DscDag::NodeToken array_children_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_ui_node_group, DscUi::TUiNodeGroup::TArrayChildUiNodeGroup);
        const auto& array_children = DscDag::GetValueNodeArray(array_children_node);
        for (auto& child : array_children)
        {
            TraverseHierarchyDeleteNode(
                child,
                in_dag_collection
            );
        }
        //in_ui_node_group->UnlinkInputs();
        in_dag_collection.DeleteNode(in_ui_node_group);
    }

} // namespace

DscUi::UiManager::UiManager(DscRender::DrawSystem& in_draw_system, DscCommon::FileSystem& in_file_system, DscDag::DagCollection& in_dag_collection)
{
    _dag_resource = DscDagRender::DagResource::Factory(&in_draw_system, &in_dag_collection);
    _render_target_pool = std::make_unique<DscRenderResource::RenderTargetPool>(DscRenderResource::s_default_pixel_alignment);

	_celtic_knot = std::make_unique<CelticKnot>(in_draw_system, in_file_system);

    //_full_quad_pos_uv
    // -1,1   1,1       0,0  1,0
    //     pos             uv
    // -1,-1  1,-1      0,1  1,1
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
                TUiPanelShaderConstantBufferVS(),
                D3D12_SHADER_VISIBILITY_VERTEX
            )
        );
        array_shader_constants_info.push_back(
            DscRenderResource::ConstantBufferInfo::Factory(
                TUiPanelShaderConstantBufferPS(),
                D3D12_SHADER_VISIBILITY_PIXEL
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

    //std::shared_ptr<DscRenderResource::Shader> _effect_burn_blot_shader = {};
    _effect_burn_blot_shader = CreateEffectShader(
        in_draw_system,
        in_file_system,
        "effect_burn_blot_vs.cso",
        "effect_burn_blot_ps.cso",
        true,
        2
    );

    _effect_burn_present_shader = CreateEffectShader(
        in_draw_system,
        in_file_system,
        "effect_burn_present_vs.cso",
        "effect_burn_present_ps.cso",
        true,
        2
    );

    _effect_blur_shader = CreateEffectShader(
        in_draw_system,
        in_file_system,
        "effect_blur_vs.cso",
        "effect_blur_ps.cso",
        false
    );

    _effect_desaturate_shader = CreateEffectShader(
        in_draw_system,
        in_file_system,
        "effect_desaturate_vs.cso",
        "effect_desaturate_ps.cso",
        true
    );

}

DscUi::UiManager::~UiManager()
{
    //nop
}


#if 1

DscDag::NodeToken DscUi::UiManager::MakeRootNode(
    const ComponentConstructionHelper& in_construction_helper,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    const std::vector<TEffectConstructionHelper>& in_effect_array
)
{
    DscDag::NodeToken result = in_dag_collection.CreateGroupEnum<DscUi::TUiRootNodeGroup, DscUi::TUiNodeGroup>(nullptr, true);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result, "root node"));

    DscDag::IDagOwner* owner = dynamic_cast<DscDag::IDagOwner*>(result);
    {
        auto frame = in_dag_collection.CreateValueNone((DscRenderResource::Frame*)(nullptr), owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(frame, "frame root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiRootNodeGroup::TFrame, frame);
    }

    {
        auto time_delta = in_dag_collection.CreateValueNotZero(0.0f, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(time_delta, "time delta root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiRootNodeGroup::TTimeDelta, time_delta);
    }

    {
        auto ui_scale = in_dag_collection.CreateValueOnValueChange(1.0f, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(ui_scale, "ui scale root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiRootNodeGroup::TUiScale, ui_scale);
    }

    {
        auto input_state = in_dag_collection.CreateValueOnSet(UiInputState(), owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(input_state, "input state root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiRootNodeGroup::TInputState, input_state);
    }

    {
        auto shader_constant_buffer = _ui_panel_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto shader_constant_buffer_node = in_dag_collection.CreateValueOnSet(shader_constant_buffer, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(shader_constant_buffer_node, "shader_constant_buffer_node"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiRootNodeGroup::TUiPanelShaderConstantBuffer, shader_constant_buffer_node);
    }

    {
        auto render_target_size = in_dag_collection.CreateValueOnValueChange(DscCommon::VectorInt2::s_zero, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(render_target_size, "render target size root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiRootNodeGroup::TRenderTargetViewportSize, render_target_size);
    }

    {
        auto ui_component_type = in_dag_collection.CreateValueOnValueChange(in_construction_helper._component_type, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(ui_component_type, "ui component type root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TUiComponentType, ui_component_type);
    }

    {
        auto array_child = in_dag_collection.CreateNodeArrayEmpty(owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(array_child, "array child root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TArrayChildUiNodeGroup, array_child);
    }

    {
        auto screen_space = in_dag_collection.CreateValueOnValueChange(DscUi::ScreenSpace(), owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(screen_space, "screen space root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TScreenSpace, screen_space);
    }

    // TAvaliableSize
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TAvaliableSize,
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TRenderTargetViewportSize)
    );

    // TRenderRequestSize
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TRenderRequestSize,
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TRenderTargetViewportSize)
    );

    // TGeometrySize
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TGeometrySize,
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TRenderTargetViewportSize)
    );

    // TGeometryOffset
    {
        auto node = in_dag_collection.CreateValueNone(DscCommon::VectorInt2::s_zero, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "geometry offset root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TGeometryOffset, node);
    }

    // TScrollPos
    {
        auto node = in_dag_collection.CreateValueNone(DscCommon::VectorFloat2::s_zero, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "scroll pos root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TScrollPos, node);
    }

    auto component_resource_node_group = DscUi::MakeComponentResourceGroup(
        in_dag_collection,
        in_construction_helper,
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TTimeDelta),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TUiScale),
        nullptr,
        result
    );
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources, component_resource_node_group);

    DscDag::NodeToken base_node = nullptr;
    auto draw_node = MakeDrawStack(
        in_construction_helper,
        in_draw_system,
        in_dag_collection,
        in_effect_array,
        result,
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TRenderTargetViewportSize),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TArrayChildUiNodeGroup),
        component_resource_node_group,
        nullptr,
        base_node
    );

    // if force draw is true, we just need to re apply the last draw step, even if nothing else has changed
    DscDag::LinkNodes(DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TRenderTargetViewportSize), draw_node);

    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TDrawNode, draw_node);
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TDrawBaseNode, base_node);

    DSC_DEBUG_ONLY(DscDag::DagNodeGroup::DebugValidate<TUiComponentResourceNodeGroup>(component_resource_node_group));
    DSC_DEBUG_ONLY(DscDag::DagNodeGroup::DebugValidate<TUiRootNodeGroup>(result));

    return result;
}

// what about when we want a child to be at an index? set child of "application layer set"? put optional index in construction helper
DscDag::NodeToken DscUi::UiManager::AddChildNode(
    const ComponentConstructionHelper& in_construction_helper,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_root_node_group,
    DscDag::NodeToken in_parent,
    const std::vector<TEffectConstructionHelper>& in_effect_array
    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
)
{
    DscDag::NodeToken result = in_dag_collection.CreateGroupEnum<DscUi::TUiNodeGroup>(nullptr, true);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result, in_debug_name));
    DscDag::IDagOwner* owner = dynamic_cast<DscDag::IDagOwner*>(result);

    //TArrayChildUiNodeGroup (BEFORE TUiComponentResources)
    {
        auto array_child = in_dag_collection.CreateNodeArrayEmpty(owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(array_child, "array child"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TArrayChildUiNodeGroup, array_child);
    }

    //TUiComponentResources
    {
        auto component_resource_node_group = MakeComponentResourceGroup(
            in_dag_collection,
            in_construction_helper,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TTimeDelta),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiScale),
            in_parent,
            result
        );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources, component_resource_node_group);
    }

    //TUiComponentType
    {
        auto ui_component_type = in_dag_collection.CreateValueOnValueChange(in_construction_helper._component_type, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(ui_component_type, "ui component type"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TUiComponentType, ui_component_type);
    }

    //TUiPanelShaderConstantBuffer
    {
        auto panel_shader_constant_buffer = _ui_panel_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto node = in_dag_collection.CreateValueNone(panel_shader_constant_buffer, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "ui panel shader constant buffer"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TUiPanelShaderConstantBuffer, node);
    }

    //calculate our avaliable size
    {
        auto node = MakeNode::MakeAvaliableSize(
            in_dag_collection,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TAvaliableSize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiScale),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TUiComponentResources),
            owner
        );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TAvaliableSize, node);
    }

    // more of a order of construction issue than a circular dependency
    if (true == in_construction_helper._has_ui_scale_by_avaliable_width)
    {
        auto component_resource = DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources);
        auto ui_scale = DscDag::DagNodeGroup::GetNodeTokenEnum(component_resource, TUiComponentResourceNodeGroup::TUiScale);
        auto avaliable_size_node = DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TAvaliableSize);
        DscDag::LinkIndexNodes(1, avaliable_size_node, ui_scale);
    }

    // calculate our desired size (for stack, this is all the contents, for text, the text render size (if width limit, limit is the avaliable size width))
    DscDag::NodeToken desired_size = MakeNode::MakeDesiredSize(
        in_construction_helper._component_type,
        in_construction_helper._desired_size_from_children_max,
        in_dag_collection,
        DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiScale),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TAvaliableSize),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TArrayChildUiNodeGroup),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources),
        owner
    );

    //TGeometrySize
    {
        auto node = MakeNode::MakeGeometrySize(
            in_dag_collection,
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TUiComponentResources),
            desired_size,
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TAvaliableSize),
            owner
        );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TGeometrySize, node);
    }

    //TGeometryOffset (after TGeometrySize as canvas child uses geometry size for attach point)
    {
        auto node = MakeNode::MakeGeometryOffset(
            in_dag_collection,
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TUiComponentResources),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TAvaliableSize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiScale),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TGeometrySize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TArrayChildUiNodeGroup),
            owner
        );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TGeometryOffset, node);
    }

    //TRenderRequestSize calculate our render request size (max desired and geometry size)
    {
        auto node = MakeNode::MakeRenderRequestSize(
            in_dag_collection,
            desired_size,
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TGeometrySize),
            owner
        );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TRenderRequestSize, node);
    }

    //TScrollPos, // where is the geometry size quad is on the render target texture
    {
        auto resource_group = DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources);
        if ((nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, TUiComponentResourceNodeGroup::THasManualScrollX)) &&
            (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, TUiComponentResourceNodeGroup::THasManualScrollY)) &&
            (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, TUiComponentResourceNodeGroup::TManualScrollX)) &&
            (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, TUiComponentResourceNodeGroup::TManualScrollY)))
        {
            DscDag::NodeToken pixel_traversal_node = MakeNode::MakeNodePixelTraversal(
                in_dag_collection,
                DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TGeometrySize),
                DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TRenderRequestSize),
                owner
            );

            auto scroll_value_node = MakeNode::MakeNodeScrollValue(
                in_dag_collection,
                resource_group,
                DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TTimeDelta),
                pixel_traversal_node,
                owner
            );
            DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TScrollPos, scroll_value_node);
        }
        else
        {
            auto node = in_dag_collection.CreateValueOnValueChange(DscCommon::VectorFloat2(0, 0), owner);
            DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "scroll pos child"));
            DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TScrollPos, node);
        }
    }

    //TScreenSpaceSize, // from top left as 0,0, what is our on screen geometry footprint. for example, this is in mouse space, so if mouse is at [500,400] we want to know if it is inside our screen space to detect rollover
    {
        auto node = MakeNode::MakeScreenSpace(
            in_dag_collection,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TScreenSpace),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TRenderRequestSize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TGeometrySize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TGeometryOffset),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TRenderRequestSize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TScrollPos),
            owner
        );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TScreenSpace, node);
    }

    DscDag::NodeToken base_node = nullptr;
    auto draw_node = MakeDrawStack(
        in_construction_helper, //TUiDrawType
        in_draw_system,
        in_dag_collection,
        in_effect_array,
        in_root_node_group,
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TRenderRequestSize),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TArrayChildUiNodeGroup),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources),
        in_parent,
        base_node
    );
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TDrawNode, draw_node);
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TDrawBaseNode, base_node);

    DSC_DEBUG_ONLY(DscDag::DagNodeGroup::DebugValidate<TUiNodeGroup>(result));

    // add the crossfade node to the base node draw to ensure that if it changes, draw is triggered (not 100% sure this is still correct logic), disable for now
    {
        auto component_resource_node_group = DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources);
        DscDag::NodeToken crossfade_node = DscDag::DagNodeGroup::GetNodeTokenEnum(component_resource_node_group, TUiComponentResourceNodeGroup::TCrossfadeNode);
        if (nullptr != crossfade_node)
        {
            DscDag::LinkNodes(crossfade_node, base_node);
        }
    }

    // add result node to parent child array
    {
        auto parent_child_array_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TArrayChildUiNodeGroup);
        DSC_ASSERT(nullptr != parent_child_array_node, "parent must have a child array");
        DscDag::NodeArrayPushBack(parent_child_array_node, result);
    }

    // add draw_node to the parent draw base
    {
        auto parent_draw_base = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TDrawBaseNode);
        DscDag::LinkNodes(draw_node, parent_draw_base);
    }

    return result;
}

/// also destroys all children, but doesn't know about parent to unlink it, so main useage is for the root node
void DscUi::UiManager::DestroyNode(
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_node_group
)
{
    TraverseHierarchyUnlink(in_node_group);
    TraverseHierarchyDeleteNode(in_node_group, in_dag_collection);

    return;
}

/// detach the child from the parent then recusivly destroy the child
void DscUi::UiManager::RemoveDestroyChild(
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_parent,
    DscDag::NodeToken in_child_to_destroy
)
{
    // remove from parent the child node to remove
    {
        auto parent_child_array_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TArrayChildUiNodeGroup);
        DSC_ASSERT(nullptr != parent_child_array_node, "parent must have a child array");
        DscDag::NodeArrayRemove(parent_child_array_node, in_child_to_destroy);
    }

    // unlink draw_node to the parent draw base
    {
        auto draw_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_child_to_destroy, TUiNodeGroup::TDrawNode);
        auto parent_draw_base = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TDrawBaseNode);
        DscDag::UnlinkNodes(draw_node, parent_draw_base);
    }

    TraverseHierarchyUnlink(in_child_to_destroy);
    TraverseHierarchyDeleteNode(in_child_to_destroy, in_dag_collection);

    return;
}

void DscUi::UiManager::Update(
    DscDag::NodeToken in_root_node_group,
    const float in_time_delta,
    const UiInputParam& in_input_param,
    const DscCommon::VectorInt2& in_layout_target_size
)
{
    DscDag::SetValueType(DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TTimeDelta), in_time_delta);

    DscDag::SetValueType(DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TRenderTargetViewportSize), in_layout_target_size);

    DscUi::ScreenSpace screen_space({ DscCommon::VectorFloat4(
        0.0f,
        0.0f,
        static_cast<float>(in_layout_target_size.GetX()),
        static_cast<float>(in_layout_target_size.GetY())
    ), DscCommon::VectorFloat4(
        0.0f,
        0.0f,
        static_cast<float>(in_layout_target_size.GetX()),
        static_cast<float>(in_layout_target_size.GetY())
    ) });

    DscDag::SetValueType(DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiNodeGroup::TScreenSpace), screen_space);

    UiInputState& input_state = DscDag::GetValueNonConstRef<UiInputState>(
        DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TInputState),
        false);

    //travers node hierarcy with the in_input_state updating a UiInputInternal to effect state/ button clicks/ rollover
    //multiple touches may also be the keyboard navigation? 
    //if there is only going to be one touch, then combine TraverseHierarchyInput and TraverseHierarchyRolloverAccumulate
    bool first = true;
    for (const auto& touch : in_input_param._touch_data_array)
    {
        bool consumed = false;
        bool ignore = false;
        TraverseHierarchyInput(
            touch,
            in_root_node_group,
            input_state.GetTouchState(touch),
            first,
            consumed,
            ignore
        );
        first = false;
    }

    TraverseHierarchyRolloverAccumulate(
        in_root_node_group,
        in_time_delta
    );

    return;
}

// return the UiRenderTarget of the full ui image, size is from Update::in_layout_target_size
DscUi::UiRenderTarget* const DscUi::UiManager::Draw(
    DscDag::NodeToken in_root_node_group,
    DscDag::DagCollection& in_dag_collection,
    DscRenderResource::Frame& in_frame
)
{
    if (nullptr != in_root_node_group)
    {
        DscDag::SetValueType(DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TFrame), &in_frame);
    }
    in_dag_collection.ResolveDirtyConditionNodes();

    DscDag::NodeToken draw_node = (nullptr != in_root_node_group) ? DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiNodeGroup::TDrawNode) : nullptr;

    DscUi::UiRenderTarget* const ui_render_target = (nullptr != draw_node) ? DscDag::GetValueType<DscUi::UiRenderTarget*>(draw_node) : nullptr;

    return ui_render_target;
}

void DscUi::UiManager::DrawUiTextureToCurrentRenderTarget(
    DscRenderResource::Frame& in_frame,
    DscDag::NodeToken in_root_node_group,
    UiRenderTarget* const in_ui_texture,
    const DscCommon::VectorFloat4& in_tint_colour
)
{
    const auto& shader_constant_buffer = DscDag::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(
        DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiPanelShaderConstantBuffer)
        );
    {
        auto& buffer = shader_constant_buffer->GetConstant<TUiPanelShaderConstantBufferVS>(0);
        buffer._pos_size[0] = -1.0f;
        buffer._pos_size[1] = 1.0f;
        buffer._pos_size[2] = 2.0f;
        buffer._pos_size[3] = 2.0f;

        buffer._uv_size[0] = 0.0f;
        buffer._uv_size[1] = 0.0f;
        const auto render_target_size = in_ui_texture->GetTextureSize();
        const auto texture_size = in_ui_texture->GetViewportSize();
        buffer._uv_size[2] = static_cast<float>(texture_size.GetX()) / static_cast<float>(render_target_size.GetX());
        buffer._uv_size[3] = static_cast<float>(texture_size.GetY()) / static_cast<float>(render_target_size.GetY());
    }
    {
        auto& buffer_tint = shader_constant_buffer->GetConstant<TUiPanelShaderConstantBufferPS>(1);
        buffer_tint._tint_colour[0] = in_tint_colour.GetX();
        buffer_tint._tint_colour[1] = in_tint_colour.GetY();
        buffer_tint._tint_colour[2] = in_tint_colour.GetZ();
        buffer_tint._tint_colour[3] = in_tint_colour.GetW();
    }

    _ui_panel_shader->SetShaderResourceViewHandle(0, in_ui_texture->GetTexture());
    in_frame.SetShader(_ui_panel_shader, shader_constant_buffer);
    in_frame.Draw(_ui_panel_geometry);

    return;
}

#else

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

DscDag::NodeToken DscUi::UiManager::MakeRootNode(
    const ComponentConstructionHelper& in_construction_helper,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    const std::shared_ptr<UiRenderTarget>& in_ui_render_target,
    const std::vector<TEffectConstructionHelper>& in_effect_array
    )
{
    DscDag::NodeToken result = in_dag_collection.CreateGroupEnum<DscUi::TUiRootNodeGroup, DscUi::TUiNodeGroup>(nullptr, true);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result, "root node"));

    DscDag::IDagOwner* owner = dynamic_cast<DscDag::IDagOwner*>(result);
    {
        auto frame = in_dag_collection.CreateValueNone((DscRenderResource::Frame*)(nullptr), owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(frame, "frame root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiRootNodeGroup::TFrame, frame);
    }

    {
        auto time_delta = in_dag_collection.CreateValueNotZero(0.0f, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(time_delta, "time delta root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiRootNodeGroup::TTimeDelta, time_delta);
    }

    {
        auto ui_scale = in_dag_collection.CreateValueOnValueChange(1.0f, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(ui_scale, "ui scale root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiRootNodeGroup::TUiScale, ui_scale);
    }

    {
        auto input_state = in_dag_collection.CreateValueOnSet(UiInputState(), owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(input_state, "input state root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiRootNodeGroup::TInputState, input_state);
    }

    {
        auto force_draw = in_dag_collection.CreateValueNotZero(false, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(force_draw, "force draw root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiRootNodeGroup::TForceDraw, force_draw);
    }

    {
        auto ui_render_target = in_dag_collection.CreateValueOnSet(in_ui_render_target, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(ui_render_target, "ui render target root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiRootNodeGroup::TUiRenderTarget, ui_render_target);
    }

    {
        auto render_target_size = in_dag_collection.CreateValueOnValueChange(DscCommon::VectorInt2::s_zero, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(render_target_size, "render target size root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiRootNodeGroup::TRenderTargetViewportSize, render_target_size);
    }

    {
        auto ui_component_type = in_dag_collection.CreateValueOnValueChange(in_construction_helper._component_type, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(ui_component_type, "ui component type root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TUiComponentType, ui_component_type);
    }

    {
        auto array_child = in_dag_collection.CreateNodeArrayEmpty(owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(array_child, "array child root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TArrayChildUiNodeGroup, array_child);
    }

    {
        auto screen_space = in_dag_collection.CreateValueOnValueChange(DscUi::ScreenSpace(), owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(screen_space, "screen space root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TScreenSpace, screen_space);
    }

    // TAvaliableSize
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TAvaliableSize,
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TRenderTargetViewportSize)
    );

    // TRenderRequestSize
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TRenderRequestSize,
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TRenderTargetViewportSize)
    );

    // TGeometrySize
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TGeometrySize,
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TRenderTargetViewportSize)
    );

    // TGeometryOffset
    {
        auto node = in_dag_collection.CreateValueNone(DscCommon::VectorInt2::s_zero, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "geometry offset root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TGeometryOffset, node);
    }

    // TScrollPos
    {
        auto node = in_dag_collection.CreateValueNone(DscCommon::VectorFloat2::s_zero, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "scroll pos root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TScrollPos, node);
    }

    auto component_resource_node_group = DscUi::MakeComponentResourceGroup(
        in_dag_collection,
        in_construction_helper,
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TTimeDelta),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TUiScale),
        nullptr,
        result
    );
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources, component_resource_node_group);

    DscDag::NodeToken base_node = nullptr;
    auto draw_node = MakeDrawStack(
        in_construction_helper,
        in_draw_system,
        in_dag_collection,
        in_effect_array,
        result,
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TUiRenderTarget),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TRenderTargetViewportSize),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TArrayChildUiNodeGroup),
        component_resource_node_group,
        nullptr,
        base_node
    );

    // if force draw is true, we just need to re apply the last draw step, even if nothing else has changed
    DscDag::LinkNodes(DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TForceDraw), draw_node);
    DscDag::LinkNodes(DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TRenderTargetViewportSize), draw_node);

    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TDrawNode, draw_node);
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TDrawBaseNode, base_node);

    DSC_DEBUG_ONLY(DscDag::DagNodeGroup::DebugValidate<TUiComponentResourceNodeGroup>(component_resource_node_group));
    DSC_DEBUG_ONLY(DscDag::DagNodeGroup::DebugValidate<TUiRootNodeGroup>(result));

    return result;
}

DscDag::NodeToken DscUi::UiManager::AddChildNode(
    const ComponentConstructionHelper& in_construction_helper,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_root_node_group,
    DscDag::NodeToken in_parent,
    const std::vector<TEffectConstructionHelper>& in_effect_array
    DSC_DEBUG_ONLY(DSC_COMMA const std::string & in_debug_name)
    )
{
    DscDag::NodeToken result = in_dag_collection.CreateGroupEnum<DscUi::TUiNodeGroup>(nullptr, true);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result, in_debug_name));
    DscDag::IDagOwner* owner = dynamic_cast<DscDag::IDagOwner*>(result);

    //TArrayChildUiNodeGroup (BEFORE TUiComponentResources)
    {
        auto array_child = in_dag_collection.CreateNodeArrayEmpty(owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(array_child, "array child"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TArrayChildUiNodeGroup, array_child);
    }

    //TUiComponentResources
    {
        auto component_resource_node_group = MakeComponentResourceGroup(
            in_dag_collection,
            in_construction_helper,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TTimeDelta),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiScale),
            in_parent,
            result
        );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources, component_resource_node_group);
    }

    //TUiComponentType
    {
        auto ui_component_type = in_dag_collection.CreateValueOnValueChange(in_construction_helper._component_type, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(ui_component_type, "ui component type"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TUiComponentType, ui_component_type);
    }

    //TUiPanelShaderConstantBuffer
    {
        auto panel_shader_constant_buffer = _ui_panel_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto node = in_dag_collection.CreateValueNone(panel_shader_constant_buffer, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "ui panel shader constant buffer"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TUiPanelShaderConstantBuffer, node);
    }

    //calculate our avaliable size
    {
        auto node = MakeNode::MakeAvaliableSize(
            in_dag_collection,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TAvaliableSize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiScale),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TUiComponentResources),
            owner
        );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TAvaliableSize, node);
    }

    // more of a order of construction issue than a circular dependency
    if (true == in_construction_helper._has_ui_scale_by_avaliable_width)
    {
        auto component_resource = DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources);
        auto ui_scale = DscDag::DagNodeGroup::GetNodeTokenEnum(component_resource, TUiComponentResourceNodeGroup::TUiScale);
        auto avaliable_size_node = DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TAvaliableSize);
        DscDag::LinkIndexNodes(1, avaliable_size_node, ui_scale);
    }

    // calculate our desired size (for stack, this is all the contents, for text, the text render size (if width limit, limit is the avaliable size width))
    DscDag::NodeToken desired_size = MakeNode::MakeDesiredSize(
        in_construction_helper._component_type,
        in_construction_helper._desired_size_from_children_max,
        in_dag_collection,
        DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiScale),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TAvaliableSize),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TArrayChildUiNodeGroup),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources),
        owner
        );

    //TGeometrySize
    {
        auto node = MakeNode::MakeGeometrySize(
            in_dag_collection,
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TUiComponentResources),
            desired_size,
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TAvaliableSize),
            owner
        );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TGeometrySize, node);
    }

    //TGeometryOffset (after TGeometrySize as canvas child uses geometry size for attach point)
    {
        auto node = MakeNode::MakeGeometryOffset(
            in_dag_collection,
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TUiComponentResources),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TAvaliableSize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiScale),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TGeometrySize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TArrayChildUiNodeGroup),
            owner
        );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TGeometryOffset, node);
    }

    //TRenderRequestSize calculate our render request size (max desired and geometry size)
    {
        auto node = MakeNode::MakeRenderRequestSize(
            in_dag_collection,
            desired_size,
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TGeometrySize),
            owner
        );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TRenderRequestSize, node);
    }

    //TScrollPos, // where is the geometry size quad is on the render target texture
    {
        auto resource_group = DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources);
        if ((nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, TUiComponentResourceNodeGroup::THasManualScrollX)) &&
            (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, TUiComponentResourceNodeGroup::THasManualScrollY)) &&
            (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, TUiComponentResourceNodeGroup::TManualScrollX)) &&
            (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, TUiComponentResourceNodeGroup::TManualScrollY)))
        {
            DscDag::NodeToken pixel_traversal_node = MakeNode::MakeNodePixelTraversal(
                in_dag_collection,
                DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TGeometrySize),
                DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TRenderRequestSize),
                owner
            );

            auto scroll_value_node = MakeNode::MakeNodeScrollValue(
                in_dag_collection,
                resource_group,
                DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TTimeDelta),
                pixel_traversal_node,
                owner
            );
            DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TScrollPos, scroll_value_node);
        }
        else
        {
            auto node = in_dag_collection.CreateValueOnValueChange(DscCommon::VectorFloat2(0, 0), owner);
            DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "scroll pos child"));
            DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TScrollPos, node);
        }
    }

    //TScreenSpaceSize, // from top left as 0,0, what is our on screen geometry footprint. for example, this is in mouse space, so if mouse is at [500,400] we want to know if it is inside our screen space to detect rollover
    {
        auto node = MakeNode::MakeScreenSpace(
            in_dag_collection,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TScreenSpace),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TRenderRequestSize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TGeometrySize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TGeometryOffset),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TRenderRequestSize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TScrollPos),
            owner
            );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TScreenSpace, node);
    }

    DscDag::NodeToken base_node = nullptr;
    auto draw_node = MakeDrawStack(
        in_construction_helper, //TUiDrawType
        in_draw_system,
        in_dag_collection,
        in_effect_array,
        in_root_node_group,
        nullptr,
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TRenderRequestSize),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TArrayChildUiNodeGroup),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources),
        in_parent,
        base_node
    );
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TDrawNode, draw_node);
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TDrawBaseNode, base_node);

    DSC_DEBUG_ONLY(DscDag::DagNodeGroup::DebugValidate<TUiNodeGroup>(result));

    // add the crossfade node to the base node draw to ensure that if it changes, draw is triggered (not 100% sure this is still correct logic), disable for now
    {
        auto component_resource_node_group = DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources);
        DscDag::NodeToken crossfade_node = DscDag::DagNodeGroup::GetNodeTokenEnum(component_resource_node_group, TUiComponentResourceNodeGroup::TCrossfadeNode);
        if (nullptr != crossfade_node)
        {
            DscDag::LinkNodes(crossfade_node, base_node);
        }
    }

    // add result node to parent child array
    {
        auto parent_child_array_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TArrayChildUiNodeGroup);
        DSC_ASSERT(nullptr != parent_child_array_node, "parent must have a child array");
        DscDag::NodeArrayPushBack(parent_child_array_node, result);
    }

    // add draw_node to the parent draw base
    {
        auto parent_draw_base = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TDrawBaseNode);
        DscDag::LinkNodes(draw_node, parent_draw_base);
    }

    return result;
}


void DscUi::UiManager::DestroyNode(
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_node_group
    )
{
    TraverseHierarchyUnlink(in_node_group);
    TraverseHierarchyDeleteNode(in_node_group, in_dag_collection);
    //in_dag_collection.DeleteNode(in_node_group);

    return;
}


void DscUi::UiManager::RemoveDestroyChild(
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_parent,
    DscDag::NodeToken in_child_to_destroy
)
{
    // remove from parent the child node to remove
    {
        auto parent_child_array_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TArrayChildUiNodeGroup);
        DSC_ASSERT(nullptr != parent_child_array_node, "parent must have a child array");
        DscDag::NodeArrayRemove(parent_child_array_node, in_child_to_destroy);
    }

    // unlink draw_node to the parent draw base
    {
        auto draw_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_child_to_destroy, TUiNodeGroup::TDrawNode);
        auto parent_draw_base = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TDrawBaseNode);
        DscDag::UnlinkNodes(draw_node, parent_draw_base);
    }

    TraverseHierarchyUnlink(in_child_to_destroy);
    TraverseHierarchyDeleteNode(in_child_to_destroy, in_dag_collection);

    return;
}


void DscUi::UiManager::Update(
    DscDag::NodeToken in_root_node_group,
    const float in_time_delta,
    const UiInputParam& in_input_param,
    DscRender::IRenderTarget* const in_external_render_target_or_null
)
{
    DscDag::SetValueType(DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TTimeDelta), in_time_delta);

    if (in_external_render_target_or_null)
    {
        DscDag::NodeToken node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiRenderTarget);
        auto render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(node);
        DSC_ASSERT(nullptr != render_target, "invalid state");
        render_target->UpdateExternalRenderTarget(in_external_render_target_or_null);
    }
    UpdateRootViewportSize(in_root_node_group);

    UiInputState& input_state = DscDag::GetValueNonConstRef<UiInputState>(
        DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TInputState), 
        false);

    //travers node hierarcy with the in_input_state updating a UiInputInternal to effect state/ button clicks/ rollover
    //multiple touches may also be the keyboard navigation? 
    //if there is only going to be one touch, then combine TraverseHierarchyInput and TraverseHierarchyRolloverAccumulate
    bool first = true;
    for (const auto& touch : in_input_param._touch_data_array)
    {
        bool consumed = false;
        TraverseHierarchyInput(
            touch,
            in_root_node_group,
            input_state.GetTouchState(touch),
            first,
            consumed
        );
        first = false;
    }

    TraverseHierarchyRolloverAccumulate(
        in_root_node_group,
        in_time_delta
    );

    return;
}

void DscUi::UiManager::Draw(
    DscDag::NodeToken in_root_node_group,
    DscDag::DagCollection& in_dag_collection,
    DscRenderResource::Frame& in_frame,
    const bool in_force_draw,
    DscRender::IRenderTarget* const in_external_render_target_or_null
)
{
    DscDag::SetValueType(DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TFrame), &in_frame);
    DscDag::SetValueType(DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TForceDraw), in_force_draw);

    if (in_external_render_target_or_null)
    {
        DscDag::NodeToken node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiRenderTarget);
        auto render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(node);
        DSC_ASSERT(nullptr != render_target, "invalid state");
        render_target->UpdateExternalRenderTarget(in_external_render_target_or_null);
    }
    UpdateRootViewportSize(in_root_node_group);

    in_dag_collection.ResolveDirtyConditionNodes();

    //in_root_node_group->Update();
    DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiNodeGroup::TDrawNode)->Update();

    return;
}

#endif

DscDag::NodeToken DscUi::UiManager::MakeDrawStack(
    const ComponentConstructionHelper& in_construction_helper,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    const std::vector<TEffectConstructionHelper>& in_effect_array,
    DscDag::NodeToken in_root_node_group,
    DscDag::NodeToken in_render_request_size_node,
    DscDag::NodeToken in_child_array_node_or_null,
    DscDag::NodeToken in_component_resource_group,
    DscDag::NodeToken in_parent,
    DscDag::NodeToken& out_base_node
)
{
    std::vector<DscDag::NodeToken> array_draw_nodes;
    std::vector<DscDag::NodeToken> effect_param_array;
    DscDag::NodeToken last_draw_node = nullptr;
    DscDag::IDagOwner* owner = dynamic_cast<DscDag::IDagOwner*>(in_component_resource_group);
    DSC_ASSERT(nullptr != owner, "invalid state");
    {
        DscDag::NodeToken ui_render_target_node = nullptr;
        {
            DscDag::NodeToken component_clear_colour = DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TClearColour);

            ui_render_target_node = MakeNode::MakeUiRenderTargetNode(
                in_draw_system, 
                *_render_target_pool, 
                in_dag_collection, 
                component_clear_colour,
                in_render_request_size_node,
                owner
                );
        }

        last_draw_node = MakeDrawNode(
            GetDrawTypeFromComponentType(in_construction_helper._component_type),
            in_draw_system,
            in_dag_collection,
            array_draw_nodes,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TFrame),
            ui_render_target_node,
            nullptr,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiScale),
            nullptr,
            nullptr,
            nullptr,
            in_child_array_node_or_null,
            in_component_resource_group
        );
        out_base_node = last_draw_node;
        array_draw_nodes.push_back(last_draw_node);
    }
    if (0 < in_effect_array.size())
    {
        for (size_t index = 0; index < in_effect_array.size(); ++index)
        {
            DscDag::NodeToken ui_render_target_node = nullptr;
            DscDag::NodeToken ui_render_target_node_b = nullptr;
            const auto& effect_data = in_effect_array[index];
            {
                DscDag::NodeToken effect_clear_colour = in_dag_collection.CreateValueOnValueChange(
                    DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.0f),
                    owner);
                DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(effect_clear_colour, "effect clear colour"));

                ui_render_target_node = MakeNode::MakeUiRenderTargetNode(
                    in_draw_system, 
                    *_render_target_pool, 
                    in_dag_collection, 
                    effect_clear_colour, 
                    in_render_request_size_node,
                    owner
                    );

                if (TUiEffectType::TEffectBurnBlot == effect_data._effect_type)
                {
                    ui_render_target_node_b = MakeNode::MakeUiRenderTargetNode(
                        in_draw_system,
                        *_render_target_pool,
                        in_dag_collection,
                        effect_clear_colour,
                        in_render_request_size_node,
                        owner
                    );
                }
            }

            DscDag::NodeToken effect_strength = nullptr;
            DscDag::NodeToken effect_param = nullptr;
            DscDag::NodeToken effect_tint = nullptr;

            // x. rolled over [0 ... 1] (1 == rolled over), y. time delta [0 ...], zw. mouse pos relative to shader in pixels, uv coords bottom left is 0,0
            if (TUiEffectType::TEffectBurnBlot == effect_data._effect_type)
            {
                DSC_ASSERT(nullptr != in_parent, "invalid state");
                MakeNode::MakeEffectParamTintBlotNode(
                    effect_param,
                    effect_tint,
                    in_dag_collection,
                    in_root_node_group,
                    in_parent,
                    effect_data,
                    owner
                );
            }
            else
            {
                MakeNode::MakeEffectParamTintNode(
                    effect_strength,
                    effect_param,
                    effect_tint,
                    in_dag_collection,
                    in_component_resource_group,
                    effect_data,
                    owner
                    );
            }

            last_draw_node = MakeDrawNode(
                GetDrawTypeFromEffectType(effect_data._effect_type),
                in_draw_system,
                in_dag_collection,
                array_draw_nodes, 
                DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TFrame),
                ui_render_target_node,
                ui_render_target_node_b,
                DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiScale),
                effect_strength,
                effect_param,
                effect_tint,
                in_child_array_node_or_null,
                in_component_resource_group
            );
            array_draw_nodes.push_back(last_draw_node);
        }
    }

    DSC_ASSERT(nullptr != last_draw_node, "invalid state");

    return last_draw_node;
}

DscDag::NodeToken DscUi::UiManager::MakeDrawNode(
    const TUiDrawType in_type,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    std::vector<DscDag::NodeToken>& in_array_input_stack,
    DscDag::NodeToken in_frame_node,
    DscDag::NodeToken in_ui_render_target_node,
    DscDag::NodeToken in_ui_render_target_node_b,
    DscDag::NodeToken in_ui_scale,
    DscDag::NodeToken in_effect_strength_or_null,
    DscDag::NodeToken in_effect_param_or_null,
    DscDag::NodeToken in_effect_tint_or_null,
    DscDag::NodeToken in_child_array_node_or_null,
    DscDag::NodeToken in_component_resource_group
)
{
    DscDag::IDagOwner* owner = dynamic_cast<DscDag::IDagOwner*>(in_component_resource_group);
    DSC_ASSERT(nullptr != owner, "invalid state");

    DSC_ASSERT(nullptr != in_frame_node, "invalid param");
    //DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "MakeDrawNode in_frame_node:%s TYPE:%s\n", in_frame_node->GetTypeInfo().name(), typeid(DscRenderResource::Frame*).name());
    DSC_ASSERT(in_frame_node->DebugGetTypeInfo() == typeid(DscRenderResource::Frame*), "invalid param");

    DSC_ASSERT(nullptr != in_ui_scale, "invalid param");
    //DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "MakeDrawNode in_ui_scale:%s TYPE:%s\n", in_ui_scale->GetTypeInfo().name(), typeid(float).name());
    DSC_ASSERT(in_ui_scale->DebugGetTypeInfo() == typeid(float), "invalid param");

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
                auto frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
                DSC_ASSERT(nullptr != frame, "invalid state");
                auto ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
                DSC_ASSERT(nullptr != ui_render_target, "invalid state");
                auto shader_buffer = DscDag::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[2]);
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
            owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result_node, "debug grid draw"));

        auto shader_buffer = _debug_grid_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto shader_buffer_node = in_dag_collection.CreateValue(
            shader_buffer,
            DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
            owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(shader_buffer_node, "shader constant"));

        DscDag::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::LinkIndexNodes(2, shader_buffer_node, result_node);
    }
    break;
    case TUiDrawType::TFill:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _full_quad_pos;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _fill_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            auto shader_buffer = DscDag::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[2]);
            DSC_ASSERT(nullptr != shader_buffer, "invalid state");
            const DscCommon::VectorFloat4& fill_colour = DscDag::GetValueType<DscCommon::VectorFloat4>(in_input_array[3]);

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
            owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result_node, "fill draw"));

        auto shader_buffer = _fill_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto shader_buffer_node = in_dag_collection.CreateValue(
            shader_buffer,
            DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
            owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(shader_buffer_node, "shader constant"));

        DscDag::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::LinkIndexNodes(2, shader_buffer_node, result_node);
        DSC_ASSERT(nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TFillColour), "invalid state");
        DscDag::LinkIndexNodes(3, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TFillColour), result_node);
    }
    break;
    case TUiDrawType::TGradientFill:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _full_quad_pos_uv;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _gradient_fill_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            auto shader_buffer = DscDag::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[2]);
            DSC_ASSERT(nullptr != shader_buffer, "invalid state");

            const TGradientFillConstantBuffer& gradient_fill = DscDag::GetValueType<TGradientFillConstantBuffer>(in_input_array[3]);
            shader_buffer->GetConstant<TGradientFillConstantBuffer>(0) = gradient_fill;

            if (true == ui_render_target->ActivateRenderTarget(*frame))
            {
                frame->SetShader(weak_shader.lock(), shader_buffer);
                frame->Draw(weak_geometry.lock());
                frame->SetRenderTarget(nullptr);
            }

            out_value = ui_render_target.get();
        },
            owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result_node, "gradient fill draw"));

        auto shader_buffer = _gradient_fill_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto shader_buffer_node = in_dag_collection.CreateValue(
            shader_buffer,
            DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
            owner);
            DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(shader_buffer_node,"shader constant"));

        DscDag::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::LinkIndexNodes(2, shader_buffer_node, result_node);
        DSC_ASSERT(nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TGradientFill), "invalid state");
        DscDag::LinkIndexNodes(3, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TGradientFill), result_node);
    }
    break;
	case TUiDrawType::TCelticKnotFill:
	{
		result_node = _celtic_knot->MakeDrawNode(
			_full_quad_pos_uv,
			in_dag_collection,
            in_draw_system,
			*_render_target_pool,
            in_frame_node,
            in_ui_render_target_node,
			in_component_resource_group
			);
	}
	break;
    case TUiDrawType::TImage:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _full_quad_pos_uv;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _image_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            auto shader_resource = DscDag::GetValueType<std::shared_ptr<DscRenderResource::ShaderResource>>(in_input_array[2]);

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
            owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result_node, "image draw"));

        auto texture_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TTexture);
        DSC_ASSERT(nullptr != texture_node, "invalid state");

        DscDag::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::LinkIndexNodes(2, texture_node, result_node);
    }
    break;
    case TUiDrawType::TUiPanel:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _ui_panel_geometry;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _ui_panel_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            const auto& ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            const std::vector<DscDag::NodeToken>& child_array = DscDag::GetValueNodeArray(in_input_array[2]);

            if (true == ui_render_target->ActivateRenderTarget(*frame))
            {
                auto shader = weak_shader.lock();
                auto geometry = weak_geometry.lock();
                const DscCommon::VectorInt2 parent_render_size = ui_render_target->GetViewportSize();

                for (const auto& child : child_array)
                {
                    DscUi::UiRenderTarget* child_render_target = DscDag::GetValueType<DscUi::UiRenderTarget*>(DscDag::DagNodeGroup::GetNodeTokenEnum(child, TUiNodeGroup::TDrawNode));
                    DSC_ASSERT(nullptr != child_render_target, "invalid state");
                    auto child_texture = child_render_target->GetTexture();
                    if (nullptr == child_texture)
                    {
                        continue;
                    }

                    const auto& shader_constant_buffer = DscDag::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(DscDag::DagNodeGroup::GetNodeTokenEnum(child, TUiNodeGroup::TUiPanelShaderConstantBuffer));

                    auto& buffer = shader_constant_buffer->GetConstant<TUiPanelShaderConstantBufferVS>(0);
                    const DscCommon::VectorInt2& geometry_offset = DscDag::GetValueType<DscCommon::VectorInt2>(DscDag::DagNodeGroup::GetNodeTokenEnum(child, TUiNodeGroup::TGeometryOffset));
                    const DscCommon::VectorInt2& geometry_size = DscDag::GetValueType<DscCommon::VectorInt2>(DscDag::DagNodeGroup::GetNodeTokenEnum(child, TUiNodeGroup::TGeometrySize));
                    const DscCommon::VectorFloat2& scroll_value = DscDag::GetValueType<DscCommon::VectorFloat2>(DscDag::DagNodeGroup::GetNodeTokenEnum(child, TUiNodeGroup::TScrollPos));
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

                    auto& buffer_tint = shader_constant_buffer->GetConstant<TUiPanelShaderConstantBufferPS>(1);
                    DscDag::NodeToken tint_token_node = DscDag::DagNodeGroup::GetNodeTokenEnum(child, DscUi::TUiNodeGroup::TUiPanelTint);
                    if (nullptr != tint_token_node)
                    {
                        const DscCommon::VectorFloat4& tint_colour = DscDag::GetValueType<DscCommon::VectorFloat4>(tint_token_node);
                        buffer_tint._tint_colour[0] = tint_colour.GetX();
                        buffer_tint._tint_colour[1] = tint_colour.GetY();
                        buffer_tint._tint_colour[2] = tint_colour.GetZ();
                        buffer_tint._tint_colour[3] = tint_colour.GetW();
                    }
                    else
                    {
                        buffer_tint._tint_colour[0] = 1.0f;
                        buffer_tint._tint_colour[1] = 1.0f;
                        buffer_tint._tint_colour[2] = 1.0f;
                        buffer_tint._tint_colour[3] = 1.0f;
                    }

                    shader->SetShaderResourceViewHandle(0, child_texture);
                    frame->SetShader(shader, shader_constant_buffer);
                    frame->Draw(geometry);
                }

                frame->SetRenderTarget(nullptr);
            }

            out_value = ui_render_target.get();
        },
            owner);

        static int32 s_index = 0;
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result_node, std::string("ui panel draw ") + std::to_string(s_index)));
        s_index += 1;

        DscDag::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DSC_ASSERT(nullptr != in_child_array_node_or_null, "invalid state");
        DscDag::LinkIndexNodes(2, in_child_array_node_or_null, result_node);
    }
    break;
    case TUiDrawType::TText:
    {
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                DscRenderResource::Frame& frame = *DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
                //DSC_ASSERT(nullptr != frame, "invalid state");
                const auto& ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
                DSC_ASSERT(nullptr != ui_render_target, "invalid state");
                const TUiComponentTextData& text_data = DscDag::GetValueType<TUiComponentTextData>(in_input_array[2]);

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
                    frame.SetRenderTarget(nullptr);
                }

                out_value = ui_render_target.get();
            },
            owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result_node, "draw text"));

        DscDag::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::LinkIndexNodes(2, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TText), result_node);

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
            in_effect_strength_or_null,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            owner
            DSC_DEBUG_ONLY(DSC_COMMA "corner"));
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
            in_effect_strength_or_null,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            owner
            DSC_DEBUG_ONLY(DSC_COMMA "drop shadow"));
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
            in_effect_strength_or_null,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            owner
            DSC_DEBUG_ONLY(DSC_COMMA "inner shadow"));
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
            in_effect_strength_or_null,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            owner
            DSC_DEBUG_ONLY(DSC_COMMA "stroke"));
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
            in_effect_strength_or_null,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            owner
            DSC_DEBUG_ONLY(DSC_COMMA "tint"));
        break;
    case TUiDrawType::TEffectBlur:
        result_node = MakeNode::MakeEffectDrawNode(
            _full_quad_pos_uv,
            _effect_blur_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_strength_or_null,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            owner
            DSC_DEBUG_ONLY(DSC_COMMA "blur"));
        break;
    case TUiDrawType::TEffectDesaturate:
        result_node = MakeNode::MakeEffectDrawNode(
            _full_quad_pos_uv,
            _effect_desaturate_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_strength_or_null,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            1,
            owner
            DSC_DEBUG_ONLY(DSC_COMMA "desaturate"));
        break;
    case TUiDrawType::TEffectBurnBlot:
    {
        result_node = MakeNode::MakeEffectBurnBlotDrawNode(
            _full_quad_pos_uv,
            _effect_burn_blot_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_render_target_node_b,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            owner
            DSC_DEBUG_ONLY(DSC_COMMA "burn blot"));
    }
        break;
    case TUiDrawType::TEffectBurnPresent:
        result_node = MakeNode::MakeEffectDrawNode(
            _full_quad_pos_uv,
            _effect_burn_present_shader,
            in_dag_collection,
            in_draw_system,
            in_frame_node,
            in_ui_render_target_node,
            in_ui_scale,
            in_effect_strength_or_null,
            in_effect_param_or_null,
            in_effect_tint_or_null,
            in_array_input_stack,
            2,
            owner
            DSC_DEBUG_ONLY(DSC_COMMA "burn present"));
        break;

    }
    return result_node;
}
