#include "ui_manager.h"
#include "screen_quad.h"
#include "ui_component_canvas.h"
#include "ui_component_debug_grid.h"
#include "ui_component_effect_round_corner.h"
#include "ui_component_fill.h"
#include "ui_component_margin.h"
#include "ui_component_padding.h"
#include "ui_component_stack.h"
#include "ui_component_text.h"
#include "ui_dag_node_component.h"
#include "ui_enum.h"
#include <dsc_common\data_helper.h>
#include <dsc_common\file_system.h>
#include <dsc_common\log_system.h>
#include <dsc_common\vector_float2.h>
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
#include <dsc_render_resource\shader_resource_info.h>

namespace
{
    static const std::vector<D3D12_INPUT_ELEMENT_DESC> s_input_element_desc_array({
        D3D12_INPUT_ELEMENT_DESC
        {
            "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, \
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 // UINT InstanceDataStepRate;
        }
        });

    DscDag::NodeToken MakeNodeGetChildAvaliableSize(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_parent_ui_component, DscDag::NodeToken in_parent_avaliable_size, DscDag::NodeToken in_child_index, DscDag::NodeToken in_ui_scale)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscUi::UiDagNodeComponent* ui_component = dynamic_cast<DscUi::UiDagNodeComponent*>(in_input_array[0]);
            DscCommon::VectorInt2 parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            const int32 child_index = DscDag::DagCollection::GetValueType<int32>(in_input_array[2]);
            const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[3]);

            value = ui_component->GetComponent().GetChildAvaliableSize(parent_avaliable_size, child_index, ui_scale);
        }
        DSC_DEBUG_ONLY(DSC_COMMA "GetChildAvaliableSize"));

        DscDag::DagCollection::LinkIndexNodes(0, in_parent_ui_component, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_parent_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_child_index, node);
        DscDag::DagCollection::LinkIndexNodes(3, in_ui_scale, node);
        
        return node;
    }

    DscDag::NodeToken MakeNodeConvertAvaliableSizeToDesiredSize(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_ui_component, DscDag::NodeToken in_parent_avaliable_size, DscDag::NodeToken in_avaliable_size, const DscDag::NodeToken in_ui_scale)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscUi::UiDagNodeComponent* ui_component = dynamic_cast<DscUi::UiDagNodeComponent*>(in_input_array[0]);
            DscCommon::VectorInt2 parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            DscCommon::VectorInt2 avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[2]);
            const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[3]);

            value = ui_component->GetComponent().ConvertAvaliableSizeToDesiredSize(parent_avaliable_size, avaliable_size, ui_scale);
        }
        DSC_DEBUG_ONLY(DSC_COMMA "ConvertAvaliableSizeToDesiredSize"));
        DscDag::DagCollection::LinkIndexNodes(0, in_ui_component, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_parent_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(3, in_ui_scale, node);

        return node;
    }

    DscDag::NodeToken MakeNodeGetChildGeometrySize(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_parent_ui_component, DscDag::NodeToken in_child_avaliable_size, DscDag::NodeToken in_child_desired_size)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscUi::UiDagNodeComponent* ui_component = dynamic_cast<DscUi::UiDagNodeComponent*>(in_input_array[0]);
            DscCommon::VectorInt2 child_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            DscCommon::VectorInt2 child_desired_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[2]);

            value = ui_component->GetComponent().GetChildGeometrySize(child_desired_size, child_avaliable_size);
        }
        DSC_DEBUG_ONLY(DSC_COMMA "GetChildGeometrySize"));

        DscDag::DagCollection::LinkIndexNodes(0, in_parent_ui_component, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_child_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_child_desired_size, node);

        return node;
    }

    DscDag::NodeToken MakeNodeGetChildGeometryOffset(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_parent_ui_component, DscDag::NodeToken in_parent_avaliable_size, DscDag::NodeToken in_parent_child_index, DscDag::NodeToken in_ui_scale)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscUi::UiDagNodeComponent* const ui_component = dynamic_cast<DscUi::UiDagNodeComponent*>(in_input_array[0]);
            const DscCommon::VectorInt2 parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            const int32 child_index = DscDag::DagCollection::GetValueType<int32>(in_input_array[2]);
            const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[3]);

            value = ui_component->GetComponent().GetChildGeometryOffset(parent_avaliable_size, child_index, ui_scale);
        }
        DSC_DEBUG_ONLY(DSC_COMMA "GetChildGeometryOffset"));
        DscDag::DagCollection::LinkIndexNodes(0, in_parent_ui_component, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_parent_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_parent_child_index, node);
        DscDag::DagCollection::LinkIndexNodes(3, in_ui_scale, node);

        return node;
    }

    DscDag::NodeToken MakeNodeCalculateRenderTarget(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_desired_size, DscDag::NodeToken in_geometry_size, DscRenderResource::RenderTargetPool* in_render_target_pool, DscRender::DrawSystem* const in_draw_system, DscDag::NodeToken in_clear_colour)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([in_render_target_pool, in_draw_system](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscCommon::VectorInt2 desired_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            const DscCommon::VectorInt2 geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            const DscCommon::VectorFloat4 clear_colour = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat4>(in_input_array[2]);

            std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture> pool_texture = {};
            if (typeid(std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture>) == value.type())
            {
                pool_texture = std::any_cast<std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture>>(value);
            }

            std::vector<DscRender::RenderTargetFormatData> target_format_data_array = {};
            target_format_data_array.push_back(
                DscRender::RenderTargetFormatData(
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    true,
                    clear_colour
                )
            );
            // desired size can not be smaller than the requested size, and this is why we now need to look at the viewport size of the render target rather than just pass down the desired size
            const DscCommon::VectorInt2 request_size(
                std::max(desired_size.GetX(), geometry_size.GetX()), 
                std::max(desired_size.GetY(), geometry_size.GetY()));
            pool_texture = in_render_target_pool->CheckOrMakeRenderTarget(
                pool_texture,
                in_draw_system,
                target_format_data_array,
                DscRender::RenderTargetDepthData(),
                request_size
                );

//#if defined(DSC_LOG)
//            DscCommon::VectorInt2 render_texture_size = pool_texture->_render_target_texture->GetSize();
//            DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "NodeCalculateRenderTarget desired_size:%d %d render_texture_size:%d %d\n", desired_size.GetX(), desired_size.GetY(), render_texture_size.GetX(), render_texture_size.GetY());
//#endif
            value = pool_texture;
        } DSC_DEBUG_ONLY(DSC_COMMA "Calculate render target"));

        DscDag::DagCollection::LinkIndexNodes(0, in_desired_size, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_geometry_size, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_clear_colour, node);

        return node;
    }

    DscDag::NodeToken MakeNodeGetRenderTargetSize(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_render_target_pool_texture)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto render_texture_pool_texture = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture>>(in_input_array[0]);

            value = render_texture_pool_texture->_render_target_texture->GetSize();
        } DSC_DEBUG_ONLY(DSC_COMMA "Render target size"));

        DscDag::DagCollection::LinkIndexNodes(0, in_render_target_pool_texture, node);

        return node;
    }

    DscDag::NodeToken MakeNodeGetRenderTargetViewportSize(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_render_target_pool_texture)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            auto render_texture_pool_texture = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture>>(in_input_array[0]);

            value = render_texture_pool_texture->_render_target_texture->GetViewportSize();
        } DSC_DEBUG_ONLY(DSC_COMMA "Render target viewport size"));

        DscDag::DagCollection::LinkIndexNodes(0, in_render_target_pool_texture, node);

        return node;
    }

    DscDag::NodeToken MakeNodeUiPanelShaderConstant(
        DscDag::DagCollection& in_dag_collection, 
        DscDag::NodeToken in_parent_render_size, 
        DscDag::NodeToken in_geometry_offset, 
        DscDag::NodeToken in_geometry_size, 
        DscDag::NodeToken in_render_viewport_size,
        DscDag::NodeToken in_render_target_size,
        DscDag::NodeToken in_scroll_value
        )
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscCommon::VectorInt2 parent_render_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            DscCommon::VectorInt2 geometry_offset = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            DscCommon::VectorInt2 geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[2]);
            DscCommon::VectorInt2 render_viewport_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[3]);
            DscCommon::VectorInt2 render_target_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[4]);
            DscCommon::VectorFloat2 scroll_value = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat2>(in_input_array[5]);

            DscUi::TUiPanelShaderConstantBuffer constant_buffer = {};
            // geometry is in range [-1 ... 1], but we want the offset relative to top left
            constant_buffer._pos_size[0] = (static_cast<float>(geometry_offset.GetX()) / static_cast<float>(parent_render_size.GetX()) * 2.0f) - 1.0f;
            constant_buffer._pos_size[1] = ((1.0f - static_cast<float>(geometry_offset.GetY()) / static_cast<float>(parent_render_size.GetY())) * 2.0f) - 1.0f;
            constant_buffer._pos_size[2] = static_cast<float>(geometry_size.GetX()) / static_cast<float>(parent_render_size.GetX()) * 2.0f;
            constant_buffer._pos_size[3] = static_cast<float>(geometry_size.GetY()) / static_cast<float>(parent_render_size.GetY()) * 2.0f;

            // atd::abs to allow ping point anim of scroll as it range [-1 ... 1] => [0 .. 1]
            const float scroll_x = std::min(1.0f, std::max(0.0f, std::abs(scroll_value.GetX())));
            constant_buffer._uv_size[0] = static_cast<float>(render_viewport_size.GetX() - geometry_size.GetX()) * scroll_x / static_cast<float>(render_target_size.GetX());
            const float scroll_y = std::min(1.0f, std::max(0.0f, std::abs(scroll_value.GetY())));
            constant_buffer._uv_size[1] = static_cast<float>(render_viewport_size.GetY() - geometry_size.GetY()) * scroll_y / static_cast<float>(render_target_size.GetY());
            constant_buffer._uv_size[2] = static_cast<float>(geometry_size.GetX()) / static_cast<float>(render_target_size.GetX());
            constant_buffer._uv_size[3] = static_cast<float>(geometry_size.GetY()) / static_cast<float>(render_target_size.GetY());
            value = constant_buffer;

            //DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "NodeUiPanelShaderConstant render_viewport_size:%d %d render_target_size:%d %d\n", render_viewport_size.GetX(), render_viewport_size.GetY(), render_target_size.GetX(), render_target_size.GetY());
        }
        DSC_DEBUG_ONLY(DSC_COMMA "UiPanelShaderConstant"));

        DscDag::DagCollection::LinkIndexNodes(0, in_parent_render_size, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_geometry_offset, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_geometry_size, node);
        DscDag::DagCollection::LinkIndexNodes(3, in_render_viewport_size, node);
        DscDag::DagCollection::LinkIndexNodes(4, in_render_target_size, node);
        DscDag::DagCollection::LinkIndexNodes(5, in_scroll_value, node);

        return node;
    }

    DscDag::NodeToken MakeNodePixelTraversal(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_geometry_size, DscDag::NodeToken in_render_size)
    {
        DscDag::NodeToken node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscCommon::VectorInt2 geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            DscCommon::VectorInt2 render_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

            value = DscCommon::VectorInt2(
                std::max(0, render_size.GetX() - geometry_size.GetX()),
                std::max(0, render_size.GetY() - geometry_size.GetY())
                );
        }
        DSC_DEBUG_ONLY(DSC_COMMA "pixel traversal"));

        DscDag::DagCollection::LinkIndexNodes(0, in_geometry_size, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_render_size, node);

        return node;
    }

    constexpr float s_scroll_pixels_per_second = 32.0f;
    constexpr float s_wrap_threashold = 1.0f; // was considering 1.25f for threashold and 2.5 for ping pong step, but only pauses at one end of the anim
    constexpr float s_wrap_step_ping_pong = 2.0f;
    DscDag::NodeToken MakeNodeScrollValue(DscDag::DagCollection& in_dag_collection, DscDag::NodeToken in_ui_component, DscDag::NodeToken in_time_delta, DscDag::NodeToken in_pixel_traversal_size_node, DscDag::NodeToken in_manual_scroll_x, DscDag::NodeToken in_manual_scroll_y)
    {
        DscDag::NodeToken condition_x = in_dag_collection.CreateCalculate([](std::any& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            // if manual scroll is false and the 0 < pixel traversal, return true
            DscUi::UiDagNodeComponent* ui_component = dynamic_cast<DscUi::UiDagNodeComponent*>(in_input_array[0]);
            DscCommon::VectorInt2 pixel_traversal = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

            out_value = ((false == ui_component->GetComponent().HasManualScrollX()) && (0 < pixel_traversal.GetX()));
        }
        DSC_DEBUG_ONLY(DSC_COMMA "scroll condition x"));
        DscDag::DagCollection::LinkIndexNodes(0, in_ui_component, condition_x);
        DscDag::DagCollection::LinkIndexNodes(1, in_pixel_traversal_size_node, condition_x);

        DscDag::NodeToken condition_y = in_dag_collection.CreateCalculate([](std::any& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            // if manual scroll is false and the 0 < pixel traversal, return true
            DscUi::UiDagNodeComponent* ui_component = dynamic_cast<DscUi::UiDagNodeComponent*>(in_input_array[0]);
            DscCommon::VectorInt2 pixel_traversal = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

            out_value = ((false == ui_component->GetComponent().HasManualScrollY()) && (0 < pixel_traversal.GetY()));
        }
        DSC_DEBUG_ONLY(DSC_COMMA "scroll condition y"));
        DscDag::DagCollection::LinkIndexNodes(0, in_ui_component, condition_y);
        DscDag::DagCollection::LinkIndexNodes(1, in_pixel_traversal_size_node, condition_y);

        DscDag::NodeToken tick_scroll_x = in_dag_collection.CreateCalculate([](std::any& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const float time_delta = DscDag::DagCollection::GetValueType<float>(in_input_array[0]);
            const float time_delta_clamped = std::max(0.0f, std::min(1.0f, time_delta));
            const int32 pixel_traversal = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]).GetX();

            float value = 0.0f;
            if (typeid(float) == out_value.type())
            {
                value = std::any_cast<float>(out_value);
            }

            if (0 < pixel_traversal)
            {
                // the max of the divisor is to avoid things bouncing too quickly on small values of pixel_traversal
                value += time_delta_clamped * s_scroll_pixels_per_second / std::max(s_scroll_pixels_per_second, static_cast<float>(pixel_traversal));
                while (s_wrap_threashold < value)
                {
                    value -= s_wrap_step_ping_pong; // pingpong, consumer of the value applies std::abs
                }
            }

            out_value = value;
        }
        DSC_DEBUG_ONLY(DSC_COMMA "scroll condition x"));
        DscDag::DagCollection::LinkIndexNodes(0, in_time_delta, tick_scroll_x);
        DscDag::DagCollection::LinkIndexNodes(1, in_pixel_traversal_size_node, tick_scroll_x);

        DscDag::NodeToken tick_scroll_y = in_dag_collection.CreateCalculate([](std::any& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const float time_delta = DscDag::DagCollection::GetValueType<float>(in_input_array[0]);
            const float time_delta_clamped = std::max(0.0f, std::min(1.0f, time_delta));
            const int32 pixel_traversal = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]).GetY();

            float value = 0.0f;
            if (typeid(float) == out_value.type())
            {
                value = std::any_cast<float>(out_value);
            }

            if (0 < pixel_traversal)
            {
                // the max of the divisor is to avoid things bouncing too quickly on small values of pixel_traversal
                value += time_delta_clamped * s_scroll_pixels_per_second / std::max(s_scroll_pixels_per_second, static_cast<float>(pixel_traversal));
                while (s_wrap_threashold < value)
                {
                    value -= s_wrap_step_ping_pong; // pingpong, consumer of the value applies std::abs
                }
            }

            out_value = value;
        }
        DSC_DEBUG_ONLY(DSC_COMMA "scroll condition y"));
        DscDag::DagCollection::LinkIndexNodes(0, in_time_delta, tick_scroll_y);
        DscDag::DagCollection::LinkIndexNodes(1, in_pixel_traversal_size_node, tick_scroll_y);

        DscDag::NodeToken scroll_x = in_dag_collection.CreateValue(std::any(float(0.0f)), DscDag::TValueChangeCondition::TOnValueChange);
        DscDag::NodeToken scroll_y = in_dag_collection.CreateValue(std::any(float(0.0f)), DscDag::TValueChangeCondition::TOnValueChange);

        in_dag_collection.CreateCondition(condition_x, tick_scroll_x, in_manual_scroll_x, scroll_x, scroll_x, "conditional scroll x");
        in_dag_collection.CreateCondition(condition_y, tick_scroll_y, in_manual_scroll_y, scroll_y, scroll_y, "conditional scroll y");

        DscDag::NodeToken result_node = in_dag_collection.CreateCalculate([](std::any& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const float x = DscDag::DagCollection::GetValueType<float>(in_input_array[0]);
            const float y = DscDag::DagCollection::GetValueType<float>(in_input_array[1]);

            value = DscCommon::VectorFloat2(x, y);
        }
        DSC_DEBUG_ONLY(DSC_COMMA "scroll"));

        DscDag::DagCollection::LinkIndexNodes(0, scroll_x, result_node);
        DscDag::DagCollection::LinkIndexNodes(1, scroll_y, result_node);

        return result_node;
    }

} // namespace

DscUi::UiManager::UiManager(DscRender::DrawSystem& in_draw_system, DscCommon::FileSystem& in_file_system, DscDag::DagCollection& in_dag_collection)
{
    _dag_resource = DscDagRender::DagResource::Factory(&in_draw_system, &in_dag_collection);
    _render_target_pool = std::make_unique<DscRenderResource::RenderTargetPool>(DscRenderResource::s_default_pixel_alignment);

    //_full_target_quad
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

        _full_target_quad = std::make_shared<DscRenderResource::GeometryGeneric>(
            &in_draw_system,
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
            ScreenQuad::GetInputElementDesc(),
            vertex_raw_data,
            4
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
                TFillConstantBuffer(),
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


    // _effect_round_corner_shader
    {
        std::vector<uint8> vertex_shader_data;
        if (false == in_file_system.LoadFile(vertex_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "effect_round_corner_vs.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load vertex shader\n");
        }
        std::vector<uint8> pixel_shader_data;
        if (false == in_file_system.LoadFile(pixel_shader_data, DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "effect_round_corner_ps.cso")))
        {
            DSC_LOG_ERROR(LOG_TOPIC_DSC_UI, "failed to load pixel shader\n");
        }

        std::vector < DXGI_FORMAT > render_target_format;
        render_target_format.push_back(DXGI_FORMAT_B8G8R8A8_UNORM);
        DscRenderResource::ShaderPipelineStateData shader_pipeline_state_data(
            ScreenQuad::GetInputElementDesc(),
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
                TEffectConstantBuffer(),
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> array_shader_resource_info;
        array_shader_resource_info.push_back(
            // data sampiler as expecting to be reading source texture at 1:1 scale
            DscRenderResource::ShaderResourceInfo::FactoryDataSampler(
                nullptr,
                D3D12_SHADER_VISIBILITY_PIXEL
            )
        );
        _effect_round_corner_shader = std::make_shared<DscRenderResource::Shader>(
            &in_draw_system,
            shader_pipeline_state_data,
            vertex_shader_data,
            std::vector<uint8_t>(),
            pixel_shader_data,
            array_shader_resource_info,
            array_shader_constants_info
            );
    }

}

DscUi::UiManager::~UiManager()
{
    //nop
}

std::unique_ptr<DscUi::IUiComponent> DscUi::UiManager::MakeComponentDebugGrid(DscRender::DrawSystem& in_draw_system)
{
    auto buffer = _debug_grid_shader->MakeShaderConstantBuffer(&in_draw_system);

    std::unique_ptr<IUiComponent> result = std::make_unique<UiComponentDebugGrid>(
        _debug_grid_shader,
        buffer,
        _full_target_quad
        );
    return result;
}

std::unique_ptr<DscUi::IUiComponent> DscUi::UiManager::MakeComponentFill()
{
    std::unique_ptr<IUiComponent> result = std::make_unique<UiComponentFill>();
    return result;
}

std::unique_ptr<DscUi::IUiComponent> DscUi::UiManager::MakeComponentCanvas()
{
    std::unique_ptr<IUiComponent> result = std::make_unique<UiComponentCanvas>(
        _ui_panel_shader,
        _ui_panel_geometry
        );
    return result;
}

std::unique_ptr<DscUi::IUiComponent> DscUi::UiManager::MakeComponentText(
    DscText::TextManager& in_text_manager,
    std::unique_ptr<DscText::TextRun>&& in_text_run,
    const TUiComponentBehaviour in_behaviour
    )
{
    std::unique_ptr<IUiComponent> result = std::make_unique<UiComponentText>(
        in_text_manager,
        std::move(in_text_run),
        in_behaviour
        );
    return result;
}

std::unique_ptr<DscUi::IUiComponent> DscUi::UiManager::MakeComponentStack(
    const UiCoord& in_item_gap,
    const TUiFlow in_ui_flow
)
{
    std::unique_ptr<IUiComponent> result = std::make_unique<UiComponentStack>(
        _ui_panel_shader,
        _ui_panel_geometry,
        in_item_gap,
        in_ui_flow
        );
    return result;
}

std::unique_ptr<DscUi::IUiComponent> DscUi::UiManager::MakeComponentMargin(
    const UiCoord& in_left,
    const UiCoord& in_top,
    const UiCoord& in_right,
    const UiCoord& in_bottom
)
{
    std::unique_ptr<IUiComponent> result = std::make_unique<UiComponentMargin>(
        _ui_panel_shader,
        _ui_panel_geometry,
        in_left,
        in_top,
        in_right,
        in_bottom
        );
    return result;
}

std::unique_ptr<DscUi::IUiComponent> DscUi::UiManager::MakeComponentPadding(
    const UiCoord& in_left,
    const UiCoord& in_top,
    const UiCoord& in_right,
    const UiCoord& in_bottom
    )
{
    std::unique_ptr<IUiComponent> result = std::make_unique<UiComponentPadding>(
        _ui_panel_shader,
        _ui_panel_geometry,
        in_left,
        in_top,
        in_right,
        in_bottom
        );
    return result;
}

std::unique_ptr<DscUi::IUiComponent> DscUi::UiManager::MakeComponentEffectRoundCorner(
    DscRender::DrawSystem& in_draw_system,
    const DscCommon::VectorFloat4& in_corner_radius
)
{
    auto buffer = _effect_round_corner_shader->MakeShaderConstantBuffer(&in_draw_system);

    std::unique_ptr<IUiComponent> result = std::make_unique<UiComponentEffectRoundCorner>(
        _effect_round_corner_shader,
        buffer,
        _full_target_quad,
        in_corner_radius
        );
    return result;
}

DscUi::DagGroupUiRootNode DscUi::UiManager::MakeUiRootNode(
    DscDag::DagCollection& in_dag_collection,
    std::unique_ptr<IUiComponent>&& in_component

    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
    )
{
    DSC_ASSERT(nullptr != in_component, "invalid param");
    DSC_ASSERT(true == in_component->IsAllowedToBeTopLevelUiComponent(), "invalid state");

    DscDag::NodeToken node_token_array[static_cast<size_t>(DscUi::TUiRootNodeGroup::TCount)] = {};

    DscDag::NodeToken frame = in_dag_collection.CreateValue(
        std::any((DscRenderResource::Frame*)nullptr), 
        DscDag::TValueChangeCondition::TNever 
        DSC_DEBUG_ONLY(DSC_COMMA "frame"));
    node_token_array[static_cast<size_t>(DscUi::TUiRootNodeGroup::TFrame)] = frame;

    node_token_array[static_cast<size_t>(DscUi::TUiRootNodeGroup::TDeviceRestore)] = _dag_resource->GetDagNodeRestored();

    DscDag::NodeToken force_draw = in_dag_collection.CreateValue(
        std::any(false),
        DscDag::TValueChangeCondition::TNotZero
        DSC_DEBUG_ONLY(DSC_COMMA "force draw"));
    node_token_array[static_cast<size_t>(DscUi::TUiRootNodeGroup::TForceDraw)] = force_draw;

    DscDag::NodeToken clear_on_draw = in_dag_collection.CreateValue(
        std::any(false),
        DscDag::TValueChangeCondition::TOnValueChange
        DSC_DEBUG_ONLY(DSC_COMMA "allow clear on draw"));
    node_token_array[static_cast<size_t>(DscUi::TUiRootNodeGroup::TAllowClearOnDraw)] = clear_on_draw;

    DscDag::NodeToken render_target = in_dag_collection.CreateValue(
        std::any((DscRender::IRenderTarget*)nullptr), 
        DscDag::TValueChangeCondition::TNever
        DSC_DEBUG_ONLY(DSC_COMMA "render target"));
    node_token_array[static_cast<size_t>(DscUi::TUiRootNodeGroup::TRenderTarget)] = render_target;

    DscDag::NodeToken render_target_viewport_size = in_dag_collection.CreateValue(
        std::any(DscCommon::VectorInt2()),
        DscDag::TValueChangeCondition::TOnValueChange
        DSC_DEBUG_ONLY(DSC_COMMA "render target viewport size"));
    node_token_array[static_cast<size_t>(DscUi::TUiRootNodeGroup::TRenderTargetViewportSize)] = render_target_viewport_size;

    DscDag::NodeToken ui_draw_scale = in_dag_collection.CreateValue(
        std::any(float(1.0f)),
        DscDag::TValueChangeCondition::TOnValueChange
        DSC_DEBUG_ONLY(DSC_COMMA "ui scale"));
    node_token_array[static_cast<size_t>(DscUi::TUiRootNodeGroup::TUiScale)] = ui_draw_scale;

    DscDag::NodeToken ui_component = nullptr;
    {
        auto node = std::make_unique<UiDagNodeComponent>(
            std::move(in_component)
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
            );
        ui_component = in_dag_collection.AddCustomNode(std::move(node));
    }
    node_token_array[static_cast<size_t>(DscUi::TUiRootNodeGroup::TUiComponent)] = ui_component;

    node_token_array[static_cast<size_t>(DscUi::TUiRootNodeGroup::TTimeDelta)] = in_dag_collection.CreateValue(
        std::any(float(0.0f)),
        DscDag::TValueChangeCondition::TNotZero
        DSC_DEBUG_ONLY(DSC_COMMA "time delta"));

    DscDag::NodeToken draw_root = in_dag_collection.CreateCalculate([](std::any&, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
        DscRenderResource::Frame* frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
        DscRender::IRenderTarget* render_target = DscDag::DagCollection::GetValueType<DscRender::IRenderTarget*>(in_input_array[1]);
        const bool clear_on_draw = DscDag::DagCollection::GetValueType<bool>(in_input_array[2]);
        const float ui_draw_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[3]);
        UiDagNodeComponent* ui_dag_node_component = dynamic_cast<UiDagNodeComponent*>(in_input_array[4]);

        frame->SetRenderTarget(render_target, clear_on_draw);
        if ((nullptr != ui_dag_node_component) && (nullptr != render_target))
        {
            ui_dag_node_component->GetComponent().Draw(*frame, *render_target, ui_draw_scale);
        }
    } DSC_DEBUG_ONLY(DSC_COMMA "draw root"));

    node_token_array[static_cast<size_t>(DscUi::TUiRootNodeGroup::TDrawRoot)] = draw_root;
    DscDag::DagCollection::LinkIndexNodes(0, frame, draw_root);
    DscDag::DagCollection::LinkIndexNodes(1, render_target, draw_root);
    DscDag::DagCollection::LinkIndexNodes(2, clear_on_draw, draw_root);
    DscDag::DagCollection::LinkIndexNodes(3, ui_draw_scale, draw_root);
    DscDag::DagCollection::LinkIndexNodes(4, ui_component, draw_root);

    // not directly consumed by the Calculate function, but used to mark it dirty
    DscDag::DagCollection::LinkIndexNodes(5, force_draw, draw_root);
    DscDag::DagCollection::LinkIndexNodes(6, render_target_viewport_size, draw_root);

    DscUi::DagGroupUiRootNode result(&in_dag_collection, node_token_array);
    DSC_ASSERT(true == result.IsValid(), "invalid result");

    return result;
}

DscUi::DagGroupUiParentNode DscUi::UiManager::ConvertUiRootNodeToParentNode(const DagGroupUiRootNode& in_ui_root_node_group)
{
    DSC_ASSERT(true == in_ui_root_node_group.IsValid(), "invalid param");

    DscUi::DagGroupUiParentNode result(in_ui_root_node_group.GetDagCollection());
    result.SetNodeToken(DscUi::TUiParentNodeGroup::TUiComponent, in_ui_root_node_group.GetNodeToken(DscUi::TUiRootNodeGroup::TUiComponent));
    result.SetNodeToken(DscUi::TUiParentNodeGroup::TUiAvaliableSize, in_ui_root_node_group.GetNodeToken(DscUi::TUiRootNodeGroup::TRenderTargetViewportSize));
    result.SetNodeToken(DscUi::TUiParentNodeGroup::TUiDesiredSize, in_ui_root_node_group.GetNodeToken(DscUi::TUiRootNodeGroup::TRenderTargetViewportSize));
    result.SetNodeToken(DscUi::TUiParentNodeGroup::TUiRenderSize, in_ui_root_node_group.GetNodeToken(DscUi::TUiRootNodeGroup::TRenderTargetViewportSize));
    result.SetNodeToken(DscUi::TUiParentNodeGroup::TUiGeometrySize, in_ui_root_node_group.GetNodeToken(DscUi::TUiRootNodeGroup::TRenderTargetViewportSize));
    result.SetNodeToken(DscUi::TUiParentNodeGroup::TDraw, in_ui_root_node_group.GetNodeToken(DscUi::TUiRootNodeGroup::TDrawRoot));
    result.SetNodeToken(DscUi::TUiParentNodeGroup::TUiPanelShaderConstant, in_ui_root_node_group.GetDagCollection()->CreateValue(
        std::any(),
        DscDag::TValueChangeCondition::TOnValueChange
        DSC_DEBUG_ONLY(DSC_COMMA "dummy shader constant for root")));

    DSC_ASSERT(true == result.IsValid(), "invalid result");
    return result;
}

DscUi::DagGroupUiParentNode DscUi::UiManager::MakeUiNode(
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    std::unique_ptr<IUiComponent>&& in_component,
    const DscCommon::VectorFloat4& in_clear_colour,

    const DagGroupUiRootNode& in_root_node,
    const DagGroupUiParentNode& in_parent_node

    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
)
{
    DscUi::DagGroupUiParentNode result(&in_dag_collection);

    DscDag::NodeToken ui_component = nullptr;
    IUiComponent* const ui_component_raw = in_component.get();
    {
        auto node = std::make_unique<UiDagNodeComponent>(std::move(in_component) 
            DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
            );
        ui_component = in_dag_collection.AddCustomNode(std::move(node));
    }
    result.SetNodeToken(TUiParentNodeGroup::TUiComponent, ui_component);

    DscDag::NodeToken parent_child_index = in_dag_collection.CreateValue(
        std::any(int32(0)),
        DscDag::TValueChangeCondition::TOnValueChange
        DSC_DEBUG_ONLY(DSC_COMMA "parent_child_index"));

    DscDag::NodeToken avaliable_size = MakeNodeGetChildAvaliableSize(
        in_dag_collection, 
        in_parent_node.GetNodeToken(TUiParentNodeGroup::TUiComponent),
        in_parent_node.GetNodeToken(TUiParentNodeGroup::TUiAvaliableSize),
        parent_child_index,
        in_root_node.GetNodeToken(TUiRootNodeGroup::TUiScale)
        );
    result.SetNodeToken(TUiParentNodeGroup::TUiAvaliableSize, avaliable_size);

    DscDag::NodeToken desired_size = MakeNodeConvertAvaliableSizeToDesiredSize(
        in_dag_collection, 
        ui_component, 
        in_parent_node.GetNodeToken(TUiParentNodeGroup::TUiAvaliableSize),
        avaliable_size,
        in_root_node.GetNodeToken(TUiRootNodeGroup::TUiScale)
        );
    result.SetNodeToken(TUiParentNodeGroup::TUiDesiredSize, desired_size);

    DscDag::NodeToken clear_colour_node = in_dag_collection.CreateValue(
        std::any(in_clear_colour),
        DscDag::TValueChangeCondition::TOnValueChange
        DSC_DEBUG_ONLY(DSC_COMMA "clear colour"));

    DscDag::NodeToken geometry_offset = MakeNodeGetChildGeometryOffset(
        in_dag_collection, 
        in_parent_node.GetNodeToken(TUiParentNodeGroup::TUiComponent),
        in_parent_node.GetNodeToken(TUiParentNodeGroup::TUiAvaliableSize),
        parent_child_index,
        in_root_node.GetNodeToken(TUiRootNodeGroup::TUiScale)
        );
    DscDag::NodeToken geometry_size = MakeNodeGetChildGeometrySize(
        in_dag_collection, 
        in_parent_node.GetNodeToken(TUiParentNodeGroup::TUiComponent),
        avaliable_size,
        desired_size
        );
    result.SetNodeToken(TUiParentNodeGroup::TUiGeometrySize, geometry_size);


    DscDag::NodeToken pixel_traversal_size_node = MakeNodePixelTraversal(in_dag_collection, geometry_size, desired_size);
    DscDag::NodeToken manual_scroll_x = in_dag_collection.CreateValue(
        std::any(float(0.0f)),
        DscDag::TValueChangeCondition::TOnValueChange
        DSC_DEBUG_ONLY(DSC_COMMA "manual scroll x"));
    DscDag::NodeToken manual_scroll_y = in_dag_collection.CreateValue(
        std::any(float(0.0f)),
        DscDag::TValueChangeCondition::TOnValueChange
        DSC_DEBUG_ONLY(DSC_COMMA "manual scroll y"));

    DscDag::NodeToken scroll_value = MakeNodeScrollValue(in_dag_collection, ui_component, in_root_node.GetNodeToken(TUiRootNodeGroup::TTimeDelta), pixel_traversal_size_node, manual_scroll_x, manual_scroll_y);

    DscDag::NodeToken render_target_pool_texture = MakeNodeCalculateRenderTarget(in_dag_collection, desired_size, geometry_size, _render_target_pool.get(), &in_draw_system, clear_colour_node);
    // reminder: render_target_pool_texture_size is the size of the full texture the render pool has given us, may be bigger than our render viewport/ desired size
    DscDag::NodeToken render_target_pool_texture_size = MakeNodeGetRenderTargetSize(in_dag_collection, render_target_pool_texture);

    DscDag::NodeToken render_target_pool_texture_viewport_size = MakeNodeGetRenderTargetViewportSize(in_dag_collection, render_target_pool_texture);

    result.SetNodeToken(TUiParentNodeGroup::TUiRenderSize, render_target_pool_texture_viewport_size);

    DscDag::NodeToken shader_constant_node = MakeNodeUiPanelShaderConstant(
        in_dag_collection, 
        in_parent_node.GetNodeToken(TUiParentNodeGroup::TUiRenderSize),
        geometry_offset,
        geometry_size, 
        render_target_pool_texture_viewport_size,
        render_target_pool_texture_size, 
        scroll_value);
    result.SetNodeToken(TUiParentNodeGroup::TUiPanelShaderConstant, shader_constant_node);

    DscDag::NodeToken draw_node = in_dag_collection.CreateCalculate([](std::any& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
        DscRenderResource::Frame* frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
        auto render_target_pool_texture = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture>>(in_input_array[1]);
        DscRender::IRenderTarget* render_target = render_target_pool_texture->_render_target_texture.get();
        const float ui_draw_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[2]);
        UiDagNodeComponent* ui_dag_node_component = dynamic_cast<UiDagNodeComponent*>(in_input_array[3]);

        frame->AddFrameResource(render_target_pool_texture->_render_target_texture);
        frame->SetRenderTarget(render_target, true);
        if ((nullptr != ui_dag_node_component) && (nullptr != render_target))
        {
            ui_dag_node_component->GetComponent().Draw(*frame, *render_target, ui_draw_scale);
        }

        //out_value = render_target_pool_texture->_render_target_texture->GetShaderResourceHeapWrapperItem(0);
        // return a std::shared_ptr<DscRenderResource::RenderTargetTexture> rather than a heap wrapper so consumer can see the texture size, the viewport size And the shader resource
        out_value = render_target_pool_texture->_render_target_texture;
    } DSC_DEBUG_ONLY(DSC_COMMA "draw node"));
    DscDag::DagCollection::LinkIndexNodes(0, in_root_node.GetNodeToken(TUiRootNodeGroup::TFrame), draw_node);
    DscDag::DagCollection::LinkIndexNodes(1, render_target_pool_texture, draw_node);
    DscDag::DagCollection::LinkIndexNodes(2, in_root_node.GetNodeToken(TUiRootNodeGroup::TUiScale), draw_node);
    DscDag::DagCollection::LinkIndexNodes(3, ui_component, draw_node);

    result.SetNodeToken(TUiParentNodeGroup::TDraw, draw_node);

    // tell the ui component about the nodes it can write to
    DagGroupUiComponent ui_component_group(&in_dag_collection);
    ui_component_group.SetNodeToken(TUiComponentGroup::TParentChildIndex, parent_child_index);
    ui_component_group.SetNodeToken(TUiComponentGroup::TClearColourNode, clear_colour_node);
    ui_component_group.SetNodeToken(TUiComponentGroup::TManualScrollX, manual_scroll_x);
    ui_component_group.SetNodeToken(TUiComponentGroup::TManualScrollY, manual_scroll_y);
    ui_component_raw->SetNode(ui_component_group);

    // not directly in the draw node calculate usage, but if it dirties, want the tree to dirty
    DscDag::DagCollection::LinkIndexNodes(4, shader_constant_node, draw_node);

    // add our draw node as an input to the parent draw
    DscDag::DagCollection::LinkNodes(draw_node, in_parent_node.GetNodeToken(TUiParentNodeGroup::TDraw));

    DSC_ASSERT(true == result.IsValid(), "invalid result");

    return result;
}

DscUi::DagGroupUiParentNode DscUi::UiManager::MakeUiNodeCanvasChild(
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    std::unique_ptr<IUiComponent>&& in_component,
    const DscCommon::VectorFloat4& in_clear_colour,

    const DagGroupUiRootNode& in_root_node,
    const DagGroupUiParentNode& in_parent_node,

    const VectorUiCoord2& in_child_size,
    const VectorUiCoord2& in_child_pivot,
    const VectorUiCoord2& in_attach_point

    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
)
{
    IUiComponent* ui_component_raw = in_component.get();

    UiDagNodeComponent* ui_dag_node_component = dynamic_cast<UiDagNodeComponent*>(in_parent_node.GetNodeToken(TUiParentNodeGroup::TUiComponent));
    UiComponentCanvas* parent_canvas = dynamic_cast<UiComponentCanvas*>(&ui_dag_node_component->GetComponent());

    auto result = MakeUiNode(
        in_draw_system,
        in_dag_collection,
        std::move(in_component),
        in_clear_colour,
        in_root_node,
        in_parent_node

        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
    );

    parent_canvas->AddChild(
        ui_component_raw, 
        in_draw_system, 
        in_child_size, 
        in_child_pivot, 
        in_attach_point, 
        result.GetNodeToken(TUiParentNodeGroup::TDraw),
        result.GetNodeToken(TUiParentNodeGroup::TUiPanelShaderConstant)
        );

    return result;
}

DscUi::DagGroupUiParentNode DscUi::UiManager::MakeUiNodeStackChild(
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    std::unique_ptr<IUiComponent>&& in_component,
    const DscCommon::VectorFloat4& in_clear_colour,

    const DagGroupUiRootNode& in_root_node,
    const DagGroupUiParentNode& in_parent_node

    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
)
{
    IUiComponent* ui_component_raw = in_component.get();

    UiDagNodeComponent* ui_dag_node_component = dynamic_cast<UiDagNodeComponent*>(in_parent_node.GetNodeToken(TUiParentNodeGroup::TUiComponent));
    UiComponentStack* parent_stack = dynamic_cast<UiComponentStack*>(&ui_dag_node_component->GetComponent());

    auto result = MakeUiNode(
        in_draw_system,
        in_dag_collection,
        std::move(in_component),
        in_clear_colour,
        in_root_node,
        in_parent_node

        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
    );

    parent_stack->AddChild(
        ui_component_raw,
        in_draw_system,
        result.GetNodeToken(TUiParentNodeGroup::TDraw),
        result.GetNodeToken(TUiParentNodeGroup::TUiPanelShaderConstant),
        result.GetNodeToken(TUiParentNodeGroup::TUiGeometrySize)
    );

    return result;
}

DscUi::DagGroupUiParentNode DscUi::UiManager::MakeUiNodeMarginChild(
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    std::unique_ptr<IUiComponent>&& in_component,
    const DscCommon::VectorFloat4& in_clear_colour,

    const DagGroupUiRootNode& in_root_node,
    const DagGroupUiParentNode& in_parent_node

    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
)
{
    IUiComponent* ui_component_raw = in_component.get();

    UiDagNodeComponent* ui_dag_node_component = dynamic_cast<UiDagNodeComponent*>(in_parent_node.GetNodeToken(TUiParentNodeGroup::TUiComponent));
    UiComponentMargin* parent_margin = dynamic_cast<UiComponentMargin*>(&ui_dag_node_component->GetComponent());

    auto result = MakeUiNode(
        in_draw_system,
        in_dag_collection,
        std::move(in_component),
        in_clear_colour,
        in_root_node,
        in_parent_node

        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
    );

    parent_margin->AddChild(
        ui_component_raw,
        in_draw_system,
        result.GetNodeToken(TUiParentNodeGroup::TDraw),
        result.GetNodeToken(TUiParentNodeGroup::TUiPanelShaderConstant)
    );

    return result;
}

DscUi::DagGroupUiParentNode DscUi::UiManager::MakeUiNodePaddingChild(
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    std::unique_ptr<IUiComponent>&& in_component,
    const DscCommon::VectorFloat4& in_clear_colour,

    const DagGroupUiRootNode& in_root_node,
    const DagGroupUiParentNode& in_parent_node

    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
)
{
    IUiComponent* ui_component_raw = in_component.get();

    UiDagNodeComponent* ui_dag_node_component = dynamic_cast<UiDagNodeComponent*>(in_parent_node.GetNodeToken(TUiParentNodeGroup::TUiComponent));
    UiComponentPadding* parent_padding = dynamic_cast<UiComponentPadding*>(&ui_dag_node_component->GetComponent());

    auto result = MakeUiNode(
        in_draw_system,
        in_dag_collection,
        std::move(in_component),
        in_clear_colour,
        in_root_node,
        in_parent_node

        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
    );

    parent_padding->AddChild(
        ui_component_raw,
        in_draw_system,
        result.GetNodeToken(TUiParentNodeGroup::TDraw),
        result.GetNodeToken(TUiParentNodeGroup::TUiPanelShaderConstant),
        result.GetNodeToken(TUiParentNodeGroup::TUiDesiredSize)
    );

    return result;
}

DscUi::DagGroupUiParentNode DscUi::UiManager::MakeUiNodeEffectRounderCornerChild(
    DscRender::DrawSystem& in_draw_system,
    DscDag::DagCollection& in_dag_collection,
    std::unique_ptr<IUiComponent>&& in_component,
    const DscCommon::VectorFloat4& in_clear_colour,

    const DagGroupUiRootNode& in_root_node,
    const DagGroupUiParentNode& in_parent_node

    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
)
{
    IUiComponent* ui_component_raw = in_component.get();

    UiDagNodeComponent* ui_dag_node_component = dynamic_cast<UiDagNodeComponent*>(in_parent_node.GetNodeToken(TUiParentNodeGroup::TUiComponent));
    UiComponentEffectRoundCorner* effect_round_corner = dynamic_cast<UiComponentEffectRoundCorner*>(&ui_dag_node_component->GetComponent());

    auto result = MakeUiNode(
        in_draw_system,
        in_dag_collection,
        std::move(in_component),
        in_clear_colour,
        in_root_node,
        in_parent_node

        DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
    );

    effect_round_corner->AddChild(
        ui_component_raw,
        result.GetNodeToken(TUiParentNodeGroup::TDraw)
    );

    return result;
}

void DscUi::UiManager::UpdateUiSystem(
    DagGroupUiRootNode& in_ui_root_node_group, // not const as setting values on it
    const float in_time_delta
    // input state? keys down, touch pos, gamepad
    )
{
    DSC_ASSERT(0.0f <= in_time_delta, "invalid param");

    {
        DscDag::NodeToken node = in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TTimeDelta);
        DscDag::DagCollection::SetValueType(node, in_time_delta);
    }

    return;
}

void DscUi::UiManager::DrawUiSystem(
    DagGroupUiRootNode& in_ui_root_node_group,
    DscRender::IRenderTarget* const in_render_target,
    DscRenderResource::Frame& in_frame,
    const bool in_force_top_level_draw, // if this render target is shared, need to at least redraw the top level ui
    const bool in_clear_on_draw, // clear the top level render target before we draw to it
    const float in_ui_scale
)
{
    DSC_ASSERT(nullptr != in_render_target, "invalid param");

    {
        DscDag::NodeToken node = in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TForceDraw);
        DscDag::DagCollection::SetValueType(node, in_force_top_level_draw);
    }
    {
        DscDag::NodeToken node = in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TAllowClearOnDraw);
        DscDag::DagCollection::SetValueType(node, in_clear_on_draw);
    }
    {
        DscDag::NodeToken node = in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TFrame);
        DscDag::DagCollection::SetValueType(node, &in_frame);
    }
    {
        DscDag::NodeToken node = in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TUiScale);
        DscDag::DagCollection::SetValueType(node, in_ui_scale);
    }
    {
        DscDag::NodeToken node = in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TRenderTarget);
        DscDag::DagCollection::SetValueType<DscRender::IRenderTarget*>(node, in_render_target);
    }
    {
        DscDag::NodeToken node = in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TRenderTargetViewportSize);
        const DscCommon::VectorInt2 viewport_size = in_render_target->GetViewportSize();
        DscDag::DagCollection::SetValueType<DscCommon::VectorInt2>(node, viewport_size);
        //DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "UiManager::DrawUiSystem viewport_size:%d %d\n", viewport_size.GetX(), viewport_size.GetY());
    }

    // after input nodes have been changed, flush the conditional state
    in_ui_root_node_group.ResolveDirtyConditionNodes();

    // trigger the graph to calculate and produce the render instructions for the ui
    in_ui_root_node_group.GetNodeToken(TUiRootNodeGroup::TDrawRoot)->GetValue();

    return;
}
