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
        case DscUi::TUiComponentType::TScrollBar:
            return DscUi::TUiDrawType::TScrollBar;
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
        const bool visible = DscDag::GetValueType<bool>(
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_ui_node_group, DscUi::TUiNodeGroup::TVisible)
            );
		if (false == visible)
		{
			return;
		}

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
			const DscCommon::VectorFloat2 node_relative_touch_pos(
				x - screen_space._screen_space.GetX(),
				y - screen_space._screen_space.GetY()
			);

            k_rollover = inside;
            bool dragged = false;
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
                       in_touch_data._node_under_click_start = in_ui_node_group;
					   in_touch_data._node_relative_click_start = node_relative_touch_pos;
                    }
                }
                else if (true == in_touch_data._click_end)
				{
					in_out_consumed = true;
                    if (in_ui_node_group == in_touch_data._node_under_click_start)
					{
						// we have a click, if we started in this node
						//flag |= DscUi::TUiInputStateFlag::TClickEnd;
						clicked = true;
					}
				}
            }

            // slightly pull this out of the above condition, saves doing it top level of callstack
            if (in_ui_node_group == in_touch_data._node_under_click_start)
            {
                if (in_touch._active)
                {
                    flag |= DscUi::TUiInputStateFlag::TClick;
					dragged = true;
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

			if ((true == clicked) || (true == dragged))
			{
				DscDag::NodeToken input_data_node = DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, DscUi::TUiComponentResourceNodeGroup::TInputData);
				if (nullptr != input_data_node)
				{
					const DscUi::TUiComponentInputData& input_data = DscDag::GetValueType<DscUi::TUiComponentInputData>(input_data_node);
					if (true == clicked)
					{
						if (nullptr != input_data._click_callback)
						{
							input_data._click_callback(in_ui_node_group, node_relative_touch_pos);
						}
					}
					else if (true == dragged)
					{
						if (nullptr != input_data._drag_callback)
						{
							// if the drag callback is triggered, tread as rollover
		                    flag |= DscUi::TUiInputStateFlag::TRollover;
							input_data._drag_callback(
								in_ui_node_group, 
								in_touch_data._node_relative_click_start,
								node_relative_touch_pos
								);
						}
					}
				}
			}

            DscDag::SetValueType<DscUi::TUiInputStateFlag>(input_state_flag, flag);

            DscDag::NodeToken active_touch_pos = DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, DscUi::TUiComponentResourceNodeGroup::TInputActiveTouchPos);
            if (nullptr != active_touch_pos)
            {
                DscDag::SetValueType(active_touch_pos, node_relative_touch_pos);
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

DscUi::UiManager::UiManager(
	DscRender::DrawSystem& in_draw_system, 
	DscCommon::FileSystem& in_file_system, 
	DscDag::DagCollection& in_dag_collection, 
	const std::vector<TEffectConstructionHelper>& in_scrollbar_effect_array
	)
	: _scrollbar_effect_array(in_scrollbar_effect_array)
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

    //_scroll_bar_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "scroll_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load triangle vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "scroll_ps.cso")))
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
                TScrollBarConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _scroll_bar_shader = std::make_shared<DscRenderResource::Shader>(
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

DscDag::NodeToken DscUi::UiManager::MakeRootNode(
    const ComponentConstructionHelper& in_construction_helper,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    const std::vector<TEffectConstructionHelper>& in_effect_array,
	const UiCoord& in_scrollbar_thickness
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
        auto render_target_size = in_dag_collection.CreateValueOnValueChange(DscCommon::VectorInt2::s_zero, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(render_target_size, "render target size root"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiRootNodeGroup::TRenderTargetViewportSize, render_target_size);
    }

	//TScrollBarThickness
	{
		auto scollbar_thickness_node = in_dag_collection.CreateCalculate<int32>([in_scrollbar_thickness]
			(int32& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
				const DscCommon::VectorInt2& viewport_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
				const float ui_scale = DscDag::GetValueType<float>(in_input_array[1]);

				out_value = in_scrollbar_thickness.Evaluate(viewport_size.GetX(), viewport_size.GetY(), ui_scale);
			}, 
			owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(scollbar_thickness_node, "scollbar_thickness_node"));
        DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TRenderTargetViewportSize), scollbar_thickness_node);
        DscDag::LinkIndexNodes(1, DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiRootNodeGroup::TUiScale), scollbar_thickness_node);

	    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiRootNodeGroup::TScrollBarThickness, scollbar_thickness_node);
	}

    {
        auto shader_constant_buffer = _ui_panel_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto shader_constant_buffer_node = in_dag_collection.CreateValueOnSet(shader_constant_buffer, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(shader_constant_buffer_node, "shader_constant_buffer_node"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TUiPanelShaderConstantBuffer, shader_constant_buffer_node);
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

	// TVisible
    {
        auto node = in_dag_collection.CreateValueOnValueChange(true, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "visible"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TVisible, node);
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
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TVisible),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TArrayChildUiNodeGroup),
        component_resource_node_group,
        nullptr,
        base_node
		DSC_DEBUG_ONLY(DSC_COMMA "root")
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
	DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "Add child node:%s\n", in_debug_name.c_str());

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

	bool add_scroll_bar_x = false;
	bool add_scroll_bar_y = false;
	DscDag::NodeToken pixel_traversal_node = nullptr;
    //TScrollPos, // where is the geometry size quad is on the render target texture
    {
        auto resource_group = DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources);
        if ((nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, TUiComponentResourceNodeGroup::THasManualScrollX)) &&
            (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, TUiComponentResourceNodeGroup::THasManualScrollY)) &&
            (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, TUiComponentResourceNodeGroup::TManualScrollX)) &&
            (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, TUiComponentResourceNodeGroup::TManualScrollY)))
        {
			add_scroll_bar_x = DscDag::GetValueType<bool>(DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, TUiComponentResourceNodeGroup::THasManualScrollX));
			add_scroll_bar_y = DscDag::GetValueType<bool>(DscDag::DagNodeGroup::GetNodeTokenEnum(resource_group, TUiComponentResourceNodeGroup::THasManualScrollY));

            pixel_traversal_node = MakeNode::MakeNodePixelTraversal(
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

	// TVisible
    {
        auto node = in_dag_collection.CreateValueOnValueChange(true, owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "visible"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TVisible, node);
    }

    DscDag::NodeToken base_node = nullptr;
    auto draw_node = MakeDrawStack(
        in_construction_helper, //TUiDrawType
        in_draw_system,
        in_dag_collection,
        in_effect_array,
        in_root_node_group,
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TRenderRequestSize),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TVisible),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TArrayChildUiNodeGroup),
        DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources),
        in_parent,
        base_node
		DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
    );
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TDrawNode, draw_node);
    DscDag::DagNodeGroup::SetNodeTokenEnum(result, TUiNodeGroup::TDrawBaseNode, base_node);

    DSC_DEBUG_ONLY(DscDag::DagNodeGroup::DebugValidate<TUiNodeGroup>(result));

    // add the crossfade node to the base node draw to ensure that if it changes, draw is triggered
    {
        auto component_resource_node_group = DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiComponentResources);
        DscDag::NodeToken crossfade_node = DscDag::DagNodeGroup::GetNodeTokenEnum(component_resource_node_group, TUiComponentResourceNodeGroup::TCrossfadeNode);
        if (nullptr != crossfade_node)
        {
            //DscDag::LinkNodes(crossfade_node, base_node);

			// actually, link it to the parent in an attempt to have the Nodefade amount of the children update BEFORE 
			// the children draw...
			auto parent_draw_base = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TDrawBaseNode);
			if (nullptr != parent_draw_base)
			{
				DscDag::LinkNodes(crossfade_node, parent_draw_base);
			}
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

        //addition dependence, ie, scroll change
        DscDag::NodeToken node = DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TGeometryOffset);
        if (nullptr != node)
        {
            DscDag::LinkNodes(node, parent_draw_base);
        }
        node = DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TGeometrySize);
        if (nullptr != node)
        {
            DscDag::LinkNodes(node, parent_draw_base);
        }
        node = DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TScrollPos);
        if (nullptr != node)
        {
            DscDag::LinkNodes(node, parent_draw_base);
        }
        node = DscDag::DagNodeGroup::GetNodeTokenEnum(result, TUiNodeGroup::TUiPanelTint);
        if (nullptr != node)
        {
            DscDag::LinkNodes(node, parent_draw_base);
        }
    }

	if ((true == add_scroll_bar_x) || 
		(true == add_scroll_bar_y))
	{
		AddScrollbar(
			in_draw_system,
			in_dag_collection,
			in_root_node_group,
			in_parent,
			result,
			pixel_traversal_node
			);
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

        //addition dependence, ie, scroll change
        DscDag::NodeToken node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_child_to_destroy, TUiNodeGroup::TGeometryOffset);
        if (nullptr != node)
        {
            DscDag::UnlinkNodes(node, parent_draw_base);
        }
        node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_child_to_destroy, TUiNodeGroup::TGeometrySize);
        if (nullptr != node)
        {
            DscDag::UnlinkNodes(node, parent_draw_base);
        }
        node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_child_to_destroy, TUiNodeGroup::TScrollPos);
        if (nullptr != node)
        {
            DscDag::UnlinkNodes(node, parent_draw_base);
        }
        node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_child_to_destroy, TUiNodeGroup::TUiPanelTint);
        if (nullptr != node)
        {
            DscDag::UnlinkNodes(node, parent_draw_base);
        }
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
	#if defined(_DEBUG)
	DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "UiManager::Draw\n\n");
	#endif// defined(_DEBUG)

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
	if (true == in_ui_texture->GetEnabled())
	{
		const auto& shader_constant_buffer = DscDag::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(
			DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiNodeGroup::TUiPanelShaderConstantBuffer)
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
	}

    return;
}

DscDag::NodeToken DscUi::UiManager::MakeDrawStack(
    const ComponentConstructionHelper& in_construction_helper,
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    const std::vector<TEffectConstructionHelper>& in_effect_array,
    DscDag::NodeToken in_root_node_group,
    DscDag::NodeToken in_render_request_size_node,
	DscDag::NodeToken in_visible,
    DscDag::NodeToken in_child_array_node_or_null,
    DscDag::NodeToken in_component_resource_group,
    DscDag::NodeToken in_parent,
    DscDag::NodeToken& out_base_node
	DSC_DEBUG_ONLY(DSC_COMMA const std::string & in_debug_name)
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
				DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
                );
        }

        last_draw_node = MakeDrawNode(
            GetDrawTypeFromComponentType(in_construction_helper._component_type),
            in_draw_system,
            in_dag_collection,
            array_draw_nodes,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TFrame),
			in_visible,
			ui_render_target_node,
            nullptr,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiScale),
            nullptr,
            nullptr,
            nullptr,
            in_child_array_node_or_null,
            in_component_resource_group
			DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
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
					DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
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
						DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
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
				in_visible,
				ui_render_target_node,
                ui_render_target_node_b,
                DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiScale),
                effect_strength,
                effect_param,
                effect_tint,
                in_child_array_node_or_null,
                in_component_resource_group
				DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
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
	DscDag::NodeToken in_visible,
    DscDag::NodeToken in_ui_render_target_node,
    DscDag::NodeToken in_ui_render_target_node_b,
    DscDag::NodeToken in_ui_scale,
    DscDag::NodeToken in_effect_strength_or_null,
    DscDag::NodeToken in_effect_param_or_null,
    DscDag::NodeToken in_effect_tint_or_null,
    DscDag::NodeToken in_child_array_node_or_null,
    DscDag::NodeToken in_component_resource_group
	DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
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
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>(
			[weak_geometry, weak_shader  DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)]
			(DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {

				#if defined(_DEBUG)
				DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "Draw node calculate debug grid:%s\n", in_debug_name.c_str());
				#endif// defined(_DEBUG)

                auto frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
                DSC_ASSERT(nullptr != frame, "invalid state");
                auto ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
                DSC_ASSERT(nullptr != ui_render_target, "invalid state");
				const bool visible = DscDag::GetValueType<bool>(in_input_array[3]);
				ui_render_target->SetEnabled(visible);
				if (true == visible)
				{
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
        DscDag::LinkIndexNodes(3, in_visible, result_node);
    }
    break;
    case TUiDrawType::TFill:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _full_quad_pos;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _fill_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>(
			[weak_geometry, weak_shader DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)]
			(DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {

			#if defined(_DEBUG)
			DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "Draw node calculate fill:%s\n", in_debug_name.c_str());
			#endif// defined(_DEBUG)

            auto frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
			const bool visible = DscDag::GetValueType<bool>(in_input_array[4]);
			ui_render_target->SetEnabled(visible);
			if (true == visible)
			{
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
        DscDag::LinkIndexNodes(4, in_visible, result_node);
    }
    break;
    case TUiDrawType::TGradientFill:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _full_quad_pos_uv;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _gradient_fill_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>(
			[weak_geometry, weak_shader  DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)]
			(DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {

			#if defined(_DEBUG)
			DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "Draw node calculate gradient fill:%s\n", in_debug_name.c_str());
			#endif// defined(_DEBUG)

            auto frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
			const bool visible = DscDag::GetValueType<bool>(in_input_array[4]);
			ui_render_target->SetEnabled(visible);
			if (true == visible)
			{
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
        DscDag::LinkIndexNodes(4, in_visible, result_node);
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
			in_component_resource_group,
			in_visible
			);
	}
	break;
    case TUiDrawType::TImage:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _full_quad_pos_uv;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _image_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>(
			[weak_geometry, weak_shader DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)]
			(DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {

			#if defined(_DEBUG)
			DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "Draw node calculate image:%s\n", in_debug_name.c_str());
			#endif// defined(_DEBUG)

            auto frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            auto ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
			const bool visible = DscDag::GetValueType<bool>(in_input_array[3]);
			ui_render_target->SetEnabled(visible);
			if (true == visible)
			{
				auto shader_resource = DscDag::GetValueType<std::shared_ptr<DscRenderResource::ShaderResource>>(in_input_array[2]);

				if (true == ui_render_target->ActivateRenderTarget(*frame))
				{
					auto shader = weak_shader.lock();
					shader->SetShaderResourceViewHandle(0, shader_resource->GetHeapWrapperItem());
					frame->SetShader(shader);
					frame->Draw(weak_geometry.lock());
					frame->SetRenderTarget(nullptr);
				}
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
        DscDag::LinkIndexNodes(3, in_visible, result_node);
    }
    break;
    case TUiDrawType::TUiPanel:
    {
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _ui_panel_geometry;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _ui_panel_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>(
			[weak_geometry, weak_shader DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)]
			(DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {

			#if defined(_DEBUG)
			DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "Draw node calculate panel:%s\n", in_debug_name.c_str());
			#endif// defined(_DEBUG)

            auto frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            const auto& ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
			const bool visible = DscDag::GetValueType<bool>(in_input_array[3]);
			ui_render_target->SetEnabled(visible);
			if (true == visible)
			{
				const std::vector<DscDag::NodeToken>& child_array = DscDag::GetValueNodeArray(in_input_array[2]);

				//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "UiPanelDraw viewport x:%d y:%d\n", ui_render_target->GetViewportSize().GetX(), ui_render_target->GetViewportSize().GetY() );

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
						if (nullptr == child_texture) // can be false if child enable is false
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
        DscDag::LinkIndexNodes(3, in_visible, result_node);

        // so, if a child's scroll value changes, we need to know to mark ourself dirty (or TUiPanelTint, TGeometryOffset, TGeometrySize ...)
        // easiest way i can think of is to just set the entire child array as a draw input, but we do actually have that as input, in_child_array_node_or_null
        // and the NodeGroup was set to not progergate member changes...
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
	case TUiDrawType::TScrollBar:
	{
        std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = _full_quad_pos;
        std::weak_ptr<DscRenderResource::Shader> weak_shader = _scroll_bar_shader;
        result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader]
			(DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                DscRenderResource::Frame& frame = *DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
                const auto& ui_render_target = DscDag::GetValueType<std::shared_ptr<UiRenderTarget>>(in_input_array[1]);
                DSC_ASSERT(nullptr != ui_render_target, "invalid state");

				const bool visible = DscDag::GetValueType<bool>(in_input_array[2]);
				ui_render_target->SetEnabled(visible);

				// if we need to draw the scroll bar
				if (true == visible)
				{
					const auto& shader_constant_buffer = DscDag::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[3]);
					const DscUi::TUiComponentScrollbarData& scroll_data = DscDag::GetValueType<DscUi::TUiComponentScrollbarData>(in_input_array[4]);
					const DscCommon::VectorFloat4& tint = DscDag::GetValueType<DscCommon::VectorFloat4>(in_input_array[5]);
					const DscCommon::VectorInt2 render_viewport_size = ui_render_target->GetViewportSize();
					auto& buffer = shader_constant_buffer->GetConstant<TScrollBarConstantBuffer>(0);
					buffer._tint[0] = tint[0];
					buffer._tint[1] = tint[1];
					buffer._tint[2] = tint[2];
					buffer._tint[3] = tint[3];
					buffer._pixel_width_height[0] = static_cast<float>(render_viewport_size.GetX());
					buffer._pixel_width_height[1] = static_cast<float>(render_viewport_size.GetY());
					buffer._pixel_low_x_y_high_x_y[0] = 0.0f;
					buffer._pixel_low_x_y_high_x_y[1] = 0.0f;
					buffer._pixel_low_x_y_high_x_y[2] = static_cast<float>(render_viewport_size.GetX());
					buffer._pixel_low_x_y_high_x_y[3] = static_cast<float>(render_viewport_size.GetY());
					if (0 != (scroll_data._scrollbar_axis_flag & TUiScrollbarAxis::THorizontal))
					{
						DSC_ASSERT(nullptr != in_input_array[6], "invalid state");
						const DscCommon::VectorFloat4& range_x = DscDag::GetValueType<DscCommon::VectorFloat4>(in_input_array[6]);
						buffer._pixel_low_x_y_high_x_y[0] = range_x[0] / range_x[2] * static_cast<float>(render_viewport_size.GetX());
						buffer._pixel_low_x_y_high_x_y[2] = range_x[1] / range_x[2] * static_cast<float>(render_viewport_size.GetX());
					}
					if (0 != (scroll_data._scrollbar_axis_flag & TUiScrollbarAxis::TVertical))
					{
						DSC_ASSERT(nullptr != in_input_array[7], "invalid state");
						const DscCommon::VectorFloat4& range_y = DscDag::GetValueType<DscCommon::VectorFloat4>(in_input_array[7]);
						buffer._pixel_low_x_y_high_x_y[1] = range_y[0] / range_y[2] * static_cast<float>(render_viewport_size.GetY());
						buffer._pixel_low_x_y_high_x_y[3] = range_y[1] / range_y[2] * static_cast<float>(render_viewport_size.GetY());
					}

					if (nullptr != in_input_array[8])
					{
						const float rollover_accumulate = DscDag::GetValueType<float>(in_input_array[8]);
						const float rollover_accumulate_clamp = DscCommon::Math::Clamp(rollover_accumulate, 0.0f, 1.0f);
						const float ratio = 1.0f - ((rollover_accumulate_clamp * 0.6f) + 0.4f);
						//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "rollover_accumulate:%f ratio:%f\n", rollover_accumulate, ratio);
						if (0 != (scroll_data._scrollbar_axis_flag & TUiScrollbarAxis::THorizontal))
						{
							buffer._pixel_low_x_y_high_x_y[1] = ((1.0f - ratio) * buffer._pixel_low_x_y_high_x_y[1]) + (ratio * buffer._pixel_low_x_y_high_x_y[3]);
						}
						if (0 != (scroll_data._scrollbar_axis_flag & TUiScrollbarAxis::TVertical))
						{
							buffer._pixel_low_x_y_high_x_y[0] = ((1.0f - ratio) * buffer._pixel_low_x_y_high_x_y[0]) + (ratio * buffer._pixel_low_x_y_high_x_y[2]);
						}
						//const float tint_ratio = rollover_accumulate_clamp * 0.5f + 0.5f;
						//buffer._tint[0] *= tint_ratio;
						//buffer._tint[1] *= tint_ratio;
						//buffer._tint[2] *= tint_ratio;
						//buffer._tint[3] *= tint_ratio;
					}

					if (true == ui_render_target->ActivateRenderTarget(frame))
					{
						auto shader = weak_shader.lock();
						auto geometry = weak_geometry.lock();
						frame.SetShader(shader, shader_constant_buffer);
						frame.Draw(geometry);
						frame.SetRenderTarget(nullptr);
					}
				}

                out_value = ui_render_target.get();
            },
            owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result_node, "draw scrollbar"));

        auto shader_buffer = _scroll_bar_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto shader_buffer_node = in_dag_collection.CreateValue(
            shader_buffer,
            DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
            owner);
            DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(shader_buffer_node,"shader constant scroll bar"));

        DscDag::LinkIndexNodes(0, in_frame_node, result_node);
        DscDag::LinkIndexNodes(1, in_ui_render_target_node, result_node);
        DscDag::LinkIndexNodes(2, in_visible, result_node);
        DscDag::LinkIndexNodes(3, shader_buffer_node, result_node);
        DscDag::LinkIndexNodes(4, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TScrollBarData), result_node);
        DscDag::LinkIndexNodes(5, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TScrollBarKnotTint), result_node);
        DscDag::LinkIndexNodes(6, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TScrollBarRangeReadX), result_node);
        DscDag::LinkIndexNodes(7, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TScrollBarRangeReadY), result_node);
        DscDag::LinkIndexNodes(8, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TInputRolloverAccumulate), result_node);
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
			in_visible,
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
			in_visible,
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
			in_visible,
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
			in_visible,
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
			in_visible,
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
			in_visible,
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
			in_visible,
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
			in_visible,
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
			in_visible,
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


void DscUi::UiManager::AddScrollbar(
	DscRender::DrawSystem& in_draw_system,
	DscDag::DagCollection& in_dag_collection,
	DscDag::NodeToken in_root_node_group,
	DscDag::NodeToken in_parent,
	DscDag::NodeToken in_node_to_scroll,
	DscDag::NodeToken in_pixel_traversal_node
	)
{
	DSC_UNUSED(in_draw_system);

	DscDag::NodeToken parent_component_resource_group =  DscDag::DagNodeGroup::GetNodeTokenEnum(in_node_to_scroll, TUiNodeGroup::TUiComponentResources);

	//display scroll x
    DscDag::NodeToken result_x = in_dag_collection.CreateGroupEnum<DscUi::TUiNodeGroup>(nullptr, true);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result_x, "scrollbar x"));
    DscDag::IDagOwner* owner_x = dynamic_cast<DscDag::IDagOwner*>(result_x);
	DscDag::NodeToken want_scroll_x = in_dag_collection.CreateCalculate<bool>(
		[]
		(bool& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
	        const DscCommon::VectorInt2& pixel_traversal = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
	        const bool has_manual_scroll = in_input_array[1] ? DscDag::GetValueType<bool>(in_input_array[1]) : false;
	        const bool visible = DscDag::GetValueType<bool>(in_input_array[2]);
			output = ((true == has_manual_scroll) && (0 < pixel_traversal.GetX()) && (true == visible));
		},
		owner_x
		);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(want_scroll_x, "want_scroll_x"));
    DscDag::LinkIndexNodes(0, in_pixel_traversal_node, want_scroll_x);
    DscDag::LinkIndexNodes(1, DscDag::DagNodeGroup::GetNodeTokenEnum(parent_component_resource_group, TUiComponentResourceNodeGroup::THasManualScrollX), want_scroll_x);
    DscDag::LinkIndexNodes(2, DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TVisible), want_scroll_x);
    DscDag::DagNodeGroup::SetNodeTokenEnum(result_x, TUiNodeGroup::TVisible, want_scroll_x);


	//display scroll y
    DscDag::NodeToken result_y = in_dag_collection.CreateGroupEnum<DscUi::TUiNodeGroup>(nullptr, true);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result_y, "scrollbar y"));
    DscDag::IDagOwner* owner_y = dynamic_cast<DscDag::IDagOwner*>(result_y);
	DscDag::NodeToken want_scroll_y = in_dag_collection.CreateCalculate<bool>(
		[]
		(bool& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
	        const DscCommon::VectorInt2& pixel_traversal = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
	        const bool has_manual_scroll = in_input_array[1] ? DscDag::GetValueType<bool>(in_input_array[1]) : false;
	        const bool visible = DscDag::GetValueType<bool>(in_input_array[2]);
			output = ((true == has_manual_scroll) && (0 < pixel_traversal.GetY()) && (true == visible));
		},
		owner_y
		);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(want_scroll_y, "want_scroll_y"));
    DscDag::LinkIndexNodes(0, in_pixel_traversal_node, want_scroll_y);
    DscDag::LinkIndexNodes(1, DscDag::DagNodeGroup::GetNodeTokenEnum(parent_component_resource_group, TUiComponentResourceNodeGroup::THasManualScrollY), want_scroll_y);
    DscDag::LinkIndexNodes(2, DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TVisible), want_scroll_y);
    DscDag::DagNodeGroup::SetNodeTokenEnum(result_y, TUiNodeGroup::TVisible, want_scroll_y);

    //TArrayChildUiNodeGroup (BEFORE TUiComponentResources)
    {
        auto array_child = in_dag_collection.CreateNodeArrayEmpty(owner_x);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(array_child, "array child"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_x, TUiNodeGroup::TArrayChildUiNodeGroup, array_child);
    }
    {
        auto array_child = in_dag_collection.CreateNodeArrayEmpty(owner_y);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(array_child, "array child"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_y, TUiNodeGroup::TArrayChildUiNodeGroup, array_child);
    }

    //TUiComponentResources X
    //{
		DscDag::NodeToken scrollbar_write_x = DscDag::DagNodeGroup::GetNodeTokenEnum(parent_component_resource_group, TUiComponentResourceNodeGroup::TManualScrollX);

		DscDag::NodeToken scrollbar_range_read_x = in_dag_collection.CreateCalculate<DscCommon::VectorFloat4>([]
		(DscCommon::VectorFloat4& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
			const DscCommon::VectorInt2& geometry_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
			const DscCommon::VectorInt2& render_request_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
			const float scroll_pos = DscDag::GetValueType<float>(in_input_array[2]);
			const float scroll_x = std::min(1.0f, std::max(0.0f, std::abs(scroll_pos)));
			const int32 overhang = std::max(0, render_request_size.GetX() - geometry_size.GetX());
			const float low = static_cast<float>(overhang) * scroll_x;
			output.Set(
				low,
				low + static_cast<float>(geometry_size.GetX()),
				static_cast<float>(render_request_size.GetX()),
				0.0f
				);
			}, owner_x
			);
		DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(scrollbar_range_read_x, "node to scroll data convertor x"));

		DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(in_node_to_scroll, TUiNodeGroup::TGeometrySize), scrollbar_range_read_x);
		DscDag::LinkIndexNodes(1, DscDag::DagNodeGroup::GetNodeTokenEnum(in_node_to_scroll, TUiNodeGroup::TRenderRequestSize), scrollbar_range_read_x);
		DscDag::LinkIndexNodes(2, scrollbar_write_x, scrollbar_range_read_x);

		auto construction_helper_x = DscUi::MakeComponentScrollbarX(
				DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 0.75f),
                DscUi::UiCoord(8, 0.0f),
				scrollbar_write_x,
				scrollbar_range_read_x
            ).SetInputData(
				nullptr,
                [scrollbar_write_x]
                (DscDag::NodeToken in_node, const DscCommon::VectorFloat2&, const DscCommon::VectorFloat2& in_node_rel_click) {
					const DscUi::ScreenSpace& screen_space = DscDag::GetValueType<DscUi::ScreenSpace>(
						DscDag::DagNodeGroup::GetNodeTokenEnum(in_node, DscUi::TUiNodeGroup::TScreenSpace)
						);
					const float length = screen_space._screen_space.GetZ() - screen_space._screen_space.GetX();
					const float ratio = (0.0f != length) ? DscCommon::Math::Clamp(in_node_rel_click.GetX() / length, 0.0f, 1.0f) : 0.0f;
					DscDag::SetValueType(scrollbar_write_x, ratio);
                    return;
                },
                true,
                true
            );

	{
        auto component_resource_node_group = MakeComponentResourceGroup(
            in_dag_collection,
            construction_helper_x,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TTimeDelta),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiScale),
            in_parent,
            result_x
        );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_x, TUiNodeGroup::TUiComponentResources, component_resource_node_group);
    }
    //TUiComponentResources Y
    //{
		DscDag::NodeToken scrollbar_write_y = DscDag::DagNodeGroup::GetNodeTokenEnum(parent_component_resource_group, TUiComponentResourceNodeGroup::TManualScrollY);

		DscDag::NodeToken scrollbar_range_read_y = in_dag_collection.CreateCalculate<DscCommon::VectorFloat4>([]
		(DscCommon::VectorFloat4& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
			const DscCommon::VectorInt2& geometry_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
			const DscCommon::VectorInt2& render_request_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
			const float scroll_pos = DscDag::GetValueType<float>(in_input_array[2]);
			const float scroll_y = std::min(1.0f, std::max(0.0f, std::abs(scroll_pos)));
			const int32 overhang = std::max(0, render_request_size.GetY() - geometry_size.GetY());
			const float low = static_cast<float>(overhang) * scroll_y;
			output.Set(
				low,
				low + static_cast<float>(geometry_size.GetY()),
				static_cast<float>(render_request_size.GetY()),
				0.0f
				);
			}, owner_y
			);
		DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(scrollbar_range_read_y, "node to scroll data convertor y"));

		DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(in_node_to_scroll, TUiNodeGroup::TGeometrySize), scrollbar_range_read_y);
		DscDag::LinkIndexNodes(1, DscDag::DagNodeGroup::GetNodeTokenEnum(in_node_to_scroll, TUiNodeGroup::TRenderRequestSize), scrollbar_range_read_y);
		DscDag::LinkIndexNodes(2, scrollbar_write_y, scrollbar_range_read_y);

		auto construction_helper_y = DscUi::MakeComponentScrollbarY(
				DscCommon::VectorFloat4(0.5f, 0.5f, 0.5f, 0.75f),
				//DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 0.1f),
                DscUi::UiCoord(8, 0.0f),
				scrollbar_write_y,
				scrollbar_range_read_y
            ).SetInputData(
				nullptr,
                [scrollbar_write_y]
                (DscDag::NodeToken in_node, const DscCommon::VectorFloat2&, const DscCommon::VectorFloat2& in_node_rel_click) {
					const DscUi::ScreenSpace& screen_space = DscDag::GetValueType<DscUi::ScreenSpace>(
						DscDag::DagNodeGroup::GetNodeTokenEnum(in_node, DscUi::TUiNodeGroup::TScreenSpace)
						);
					const float length = screen_space._screen_space.GetW() - screen_space._screen_space.GetY();
					const float ratio = (0.0f != length) ? DscCommon::Math::Clamp(in_node_rel_click.GetY() / length, 0.0f, 1.0f) : 0.0f;
					DscDag::SetValueType(scrollbar_write_y, ratio);
                    return;
                },
                true,
                true
            );
	{
        auto component_resource_node_group = MakeComponentResourceGroup(
            in_dag_collection,
            construction_helper_y,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TTimeDelta),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TUiScale),
            in_parent,
            result_y
        );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_y, TUiNodeGroup::TUiComponentResources, component_resource_node_group);
    }

    //TUiComponentType
    {
        auto ui_component_type = in_dag_collection.CreateValueOnValueChange(TUiComponentType::TScrollBar, owner_x);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(ui_component_type, "ui component type x"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_x, TUiNodeGroup::TUiComponentType, ui_component_type);
    }
    {
        auto ui_component_type = in_dag_collection.CreateValueOnValueChange(TUiComponentType::TScrollBar, owner_y);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(ui_component_type, "ui component type y"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_y, TUiNodeGroup::TUiComponentType, ui_component_type);
    }

    //TUiPanelShaderConstantBuffer
    {
        auto panel_shader_constant_buffer = _ui_panel_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto node = in_dag_collection.CreateValueNone(panel_shader_constant_buffer, owner_x);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "ui panel shader constant buffer x"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_x, TUiNodeGroup::TUiPanelShaderConstantBuffer, node);
    }
    {
        auto panel_shader_constant_buffer = _ui_panel_shader->MakeShaderConstantBuffer(&in_draw_system);
        auto node = in_dag_collection.CreateValueNone(panel_shader_constant_buffer, owner_y);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "ui panel shader constant buffer y"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_y, TUiNodeGroup::TUiPanelShaderConstantBuffer, node);
    }

	//TGeometrySize
	{
		DscDag::NodeToken geometry_size = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([]
		(DscCommon::VectorInt2& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
			const DscCommon::VectorInt2& parent_geometry_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
			const int32 scrollbar_thickness = DscDag::GetValueType<int32>(in_input_array[1]);
			const bool y_visible = DscDag::GetValueType<bool>(in_input_array[2]);
			output.Set(
				parent_geometry_size.GetX() - (y_visible ? scrollbar_thickness : 0),
				scrollbar_thickness
				);
			},
			owner_x);
		DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(geometry_size, "scrollbar size x"));
		DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(in_node_to_scroll, TUiNodeGroup::TGeometrySize), geometry_size);
		DscDag::LinkIndexNodes(1, DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TScrollBarThickness), geometry_size);
		DscDag::LinkIndexNodes(2, DscDag::DagNodeGroup::GetNodeTokenEnum(result_y, TUiNodeGroup::TVisible), geometry_size);

        DscDag::DagNodeGroup::SetNodeTokenEnum(result_x, TUiNodeGroup::TGeometrySize, geometry_size);
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_x, TUiNodeGroup::TRenderRequestSize, geometry_size);
	}
	{
		DscDag::NodeToken geometry_size = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([]
		(DscCommon::VectorInt2& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
			const DscCommon::VectorInt2& parent_geometry_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
			const int32 scrollbar_thickness = DscDag::GetValueType<int32>(in_input_array[1]);
			const bool x_visible = DscDag::GetValueType<bool>(in_input_array[2]);
			output.Set(
				scrollbar_thickness,
				parent_geometry_size.GetY() - (x_visible ? scrollbar_thickness : 0)
				);
			},
			owner_y);
		DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(geometry_size, "scrollbar size y"));
		DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(in_node_to_scroll, TUiNodeGroup::TGeometrySize), geometry_size);
		DscDag::LinkIndexNodes(1, DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TScrollBarThickness), geometry_size);
		DscDag::LinkIndexNodes(2, DscDag::DagNodeGroup::GetNodeTokenEnum(result_x, TUiNodeGroup::TVisible), geometry_size);

        DscDag::DagNodeGroup::SetNodeTokenEnum(result_y, TUiNodeGroup::TGeometrySize, geometry_size);
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_y, TUiNodeGroup::TRenderRequestSize, geometry_size);
	}

	//TGeometryOffset
	{
		DscDag::NodeToken geometry_offset = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([]
		(DscCommon::VectorInt2& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
			const DscCommon::VectorInt2& parent_geometry_offset = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
			const DscCommon::VectorInt2& parent_geometry_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
			const int32 scrollbar_thickness = DscDag::GetValueType<int32>(in_input_array[2]);
			output.Set(
				parent_geometry_offset.GetX(),
				parent_geometry_offset.GetY() + parent_geometry_size.GetY() - scrollbar_thickness
				);
			},
			owner_x);
		DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(geometry_offset, "geometry_offset x"));
		DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(in_node_to_scroll, TUiNodeGroup::TGeometryOffset), geometry_offset);
		DscDag::LinkIndexNodes(1, DscDag::DagNodeGroup::GetNodeTokenEnum(in_node_to_scroll, TUiNodeGroup::TGeometrySize), geometry_offset);
		DscDag::LinkIndexNodes(2, DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TScrollBarThickness), geometry_offset);

        DscDag::DagNodeGroup::SetNodeTokenEnum(result_x, TUiNodeGroup::TGeometryOffset, geometry_offset);
	}
	{
		DscDag::NodeToken geometry_offset = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([]
		(DscCommon::VectorInt2& output, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
			const DscCommon::VectorInt2& parent_geometry_offset = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
			const DscCommon::VectorInt2& parent_geometry_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
			const int32 scrollbar_thickness = DscDag::GetValueType<int32>(in_input_array[2]);
			output.Set(
				parent_geometry_offset.GetX() + parent_geometry_size.GetX() - scrollbar_thickness,
				parent_geometry_offset.GetY()
				);
			},
			owner_y);
		DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(geometry_offset, "geometry_offset y"));
		DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(in_node_to_scroll, TUiNodeGroup::TGeometryOffset), geometry_offset);
		DscDag::LinkIndexNodes(1, DscDag::DagNodeGroup::GetNodeTokenEnum(in_node_to_scroll, TUiNodeGroup::TGeometrySize), geometry_offset);
		DscDag::LinkIndexNodes(2, DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TScrollBarThickness), geometry_offset);

        DscDag::DagNodeGroup::SetNodeTokenEnum(result_y, TUiNodeGroup::TGeometryOffset, geometry_offset);
	}

	//TScrollPos
    {
        auto node = in_dag_collection.CreateValueNone(DscCommon::VectorFloat2::s_zero, owner_x);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "scroll pos x"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_x, TUiNodeGroup::TScrollPos, node);
    }
    {
        auto node = in_dag_collection.CreateValueNone(DscCommon::VectorFloat2::s_zero, owner_y);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "scroll pos y"));
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_y, TUiNodeGroup::TScrollPos, node);
    }

	//TScreenSpaceSize, // from top left as 0,0, what is our on screen geometry footprint. for example, this is in mouse space, so if mouse is at [500,400] we want to know if it is inside our screen space to detect rollover
    {
        auto node = MakeNode::MakeScreenSpace(
            in_dag_collection,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TScreenSpace),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TRenderRequestSize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result_x, TUiNodeGroup::TGeometrySize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result_x, TUiNodeGroup::TGeometryOffset),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result_x, TUiNodeGroup::TRenderRequestSize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result_x, TUiNodeGroup::TScrollPos),
            owner_x
        );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_x, TUiNodeGroup::TScreenSpace, node);
    }
    {
        auto node = MakeNode::MakeScreenSpace(
            in_dag_collection,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TScreenSpace),
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TRenderRequestSize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result_y, TUiNodeGroup::TGeometrySize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result_y, TUiNodeGroup::TGeometryOffset),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result_y, TUiNodeGroup::TRenderRequestSize),
            DscDag::DagNodeGroup::GetNodeTokenEnum(result_y, TUiNodeGroup::TScrollPos),
            owner_y
        );
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_y, TUiNodeGroup::TScreenSpace, node);
    }

	//TAvaliableSize
	{
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_x, TUiNodeGroup::TAvaliableSize, 
			DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TAvaliableSize)
			);
        DscDag::DagNodeGroup::SetNodeTokenEnum(result_y, TUiNodeGroup::TAvaliableSize, 
			DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TAvaliableSize)
			);
	}

	//draw
	{
		DscDag::NodeToken base_node = nullptr;
		auto draw_node = MakeDrawStack(
			construction_helper_x, //TUiDrawType
			in_draw_system,
			in_dag_collection,
			_scrollbar_effect_array,
			in_root_node_group,
			DscDag::DagNodeGroup::GetNodeTokenEnum(result_x, TUiNodeGroup::TRenderRequestSize),
			DscDag::DagNodeGroup::GetNodeTokenEnum(result_x, TUiNodeGroup::TVisible),
			DscDag::DagNodeGroup::GetNodeTokenEnum(result_x, TUiNodeGroup::TArrayChildUiNodeGroup),
			DscDag::DagNodeGroup::GetNodeTokenEnum(result_x, TUiNodeGroup::TUiComponentResources),
			in_parent,
			base_node
			DSC_DEBUG_ONLY(DSC_COMMA "scrollbar x")
		);
		DscDag::DagNodeGroup::SetNodeTokenEnum(result_x, TUiNodeGroup::TDrawNode, draw_node);
		DscDag::DagNodeGroup::SetNodeTokenEnum(result_x, TUiNodeGroup::TDrawBaseNode, base_node);

		DSC_DEBUG_ONLY(DscDag::DagNodeGroup::DebugValidate<TUiNodeGroup>(result_x));

		// add result node to parent child array
		{
			auto parent_child_array_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TArrayChildUiNodeGroup);
			DSC_ASSERT(nullptr != parent_child_array_node, "parent must have a child array");
			DscDag::NodeArrayPushBack(parent_child_array_node, result_x);
		}

		// add draw node to parent draw node
        auto parent_draw_base = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TDrawBaseNode);
        DscDag::LinkNodes(draw_node, parent_draw_base);
	}
	{
		DscDag::NodeToken base_node = nullptr;
		auto draw_node = MakeDrawStack(
			construction_helper_y, //TUiDrawType
			in_draw_system,
			in_dag_collection,
			_scrollbar_effect_array,
			in_root_node_group,
			DscDag::DagNodeGroup::GetNodeTokenEnum(result_y, TUiNodeGroup::TRenderRequestSize),
			DscDag::DagNodeGroup::GetNodeTokenEnum(result_y, TUiNodeGroup::TVisible),
			DscDag::DagNodeGroup::GetNodeTokenEnum(result_y, TUiNodeGroup::TArrayChildUiNodeGroup),
			DscDag::DagNodeGroup::GetNodeTokenEnum(result_y, TUiNodeGroup::TUiComponentResources),
			in_parent,
			base_node
			DSC_DEBUG_ONLY(DSC_COMMA "scrollbar y")
		);
		DscDag::DagNodeGroup::SetNodeTokenEnum(result_y, TUiNodeGroup::TDrawNode, draw_node);
		DscDag::DagNodeGroup::SetNodeTokenEnum(result_y, TUiNodeGroup::TDrawBaseNode, base_node);

		DSC_DEBUG_ONLY(DscDag::DagNodeGroup::DebugValidate<TUiNodeGroup>(result_y));

		// add result node to parent child array
		{
			auto parent_child_array_node = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TArrayChildUiNodeGroup);
			DSC_ASSERT(nullptr != parent_child_array_node, "parent must have a child array");
			DscDag::NodeArrayPushBack(parent_child_array_node, result_y);
		}

		// add draw node to parent draw node
        auto parent_draw_base = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent, TUiNodeGroup::TDrawBaseNode);
        DscDag::LinkNodes(draw_node, parent_draw_base);
	}

	return;
}


