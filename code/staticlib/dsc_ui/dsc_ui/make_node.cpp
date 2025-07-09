#include "make_node.h"

#include "component_construction_helper.h"
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
    /// note: the automatic scroll, to do pingpoing without state, traverses a range of [-1 ... 1] and is passed through an abs() function
    /// otherwise would need state of going up or going down and swap at [0, 1]
    /// this works, but manual scroll needs to clamp it's range [0 ... 1] else in negative domain, will be pingpong to positive....
    constexpr float s_scroll_pixels_per_second = 32.0f;
    constexpr float s_wrap_threashold = 1.0f; // was considering 1.25f for threashold and 2.5 for ping pong step, but only pauses at one end of the anim
    constexpr float s_wrap_step_ping_pong = 2.0f;
}

DscDag::NodeToken DscUi::MakeNode::MakeUiRenderTargetNode(
    DscRender::DrawSystem& in_draw_system,
    DscRenderResource::RenderTargetPool& in_render_target_pool,
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_clear_colour,
    DscDag::NodeToken in_request_size_node,
    DscUi::UiComponentResourceNodeGroup& in_component_resource_group
    )
{
    DscDag::NodeToken node = in_dag_collection.CreateCalculate<std::shared_ptr<DscUi::UiRenderTarget>>([&in_render_target_pool, &in_draw_system](std::shared_ptr<DscUi::UiRenderTarget>& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscCommon::VectorInt2 request_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            const DscCommon::VectorFloat4 clear_colour = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat4>(in_input_array[1]);

            if (nullptr == value)
            {
                value = std::make_shared<DscUi::UiRenderTarget>(true);
            }

            std::vector<DscRender::RenderTargetFormatData> target_format_data_array = {};
            target_format_data_array.push_back(
                DscRender::RenderTargetFormatData(
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    true,
                    clear_colour
                )
            );

            value->UpdateRenderTargetPool(
                in_draw_system,
                in_render_target_pool,
                request_size,
                clear_colour
                );
        }, 
        &in_component_resource_group
        DSC_DEBUG_ONLY(DSC_COMMA "ui render target"));

    DscDag::DagCollection::LinkIndexNodes(0, in_request_size_node, node);
    DscDag::DagCollection::LinkIndexNodes(1, in_clear_colour, node);

    return node;
}

DscDag::NodeToken DscUi::MakeNode::MakeEffectDrawNode(
    const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_geometry,
    const std::shared_ptr<DscRenderResource::Shader>& in_shader,
    DscDag::DagCollection& in_dag_collection,
    DscRender::DrawSystem& in_draw_system,
    DscDag::NodeToken in_frame_node,
    DscDag::NodeToken in_ui_render_target_node,
    DscDag::NodeToken in_ui_scale,
    DscDag::NodeToken in_effect_param,
    DscDag::NodeToken in_effect_tint,
    const std::vector<DscDag::NodeToken>& in_array_input_stack,
    const int32 in_input_texture_count,
    DscUi::UiComponentResourceNodeGroup& in_component_resource_group
    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name))
{
    DSC_ASSERT(nullptr != in_geometry, "invalid param");
    DSC_ASSERT(nullptr != in_shader, "invalid param");

    std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = in_geometry;
    std::weak_ptr<DscRenderResource::Shader> weak_shader = in_shader;
    DscDag::NodeToken result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader, in_input_texture_count](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            DscRenderResource::Frame* const frame = DscDag::DagCollection::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
            DSC_ASSERT(nullptr != frame, "invalid state");
            const auto& ui_render_target = DscDag::DagCollection::GetValueType<std::shared_ptr<DscUi::UiRenderTarget>>(in_input_array[1]);
            DSC_ASSERT(nullptr != ui_render_target, "invalid state");
            const auto& shader_buffer = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[2]);
            DSC_ASSERT(nullptr != shader_buffer, "invalid state");
            const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[3]);
            const DscCommon::VectorFloat4& effect_param = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat4>(in_input_array[4]);
            const DscCommon::VectorFloat4& effect_tint = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat4>(in_input_array[5]);

            const DscCommon::VectorInt2 viewport_size = ui_render_target->GetViewportSize();

            auto& buffer = shader_buffer->GetConstant<DscUi::TEffectConstantBuffer>(0);
            buffer._width_height[0] = static_cast<float>(viewport_size.GetX());
            buffer._width_height[1] = static_cast<float>(viewport_size.GetY());
            buffer._effect_param[0] = effect_param[0] * ui_scale;
            buffer._effect_param[1] = effect_param[1] * ui_scale;
            buffer._effect_param[2] = effect_param[2] * ui_scale;
            buffer._effect_param[3] = effect_param[3] * ui_scale;
            buffer._effect_tint[0] = effect_tint[0];
            buffer._effect_tint[1] = effect_tint[1];
            buffer._effect_tint[2] = effect_tint[2];
            buffer._effect_tint[3] = effect_tint[3];

            std::shared_ptr<DscRenderResource::Shader> shader = weak_shader.lock();
            DSC_ASSERT(nullptr != shader, "invalid state");

            if (0 < in_input_texture_count)
            {
                DscUi::UiRenderTarget* const input_texture = DscDag::DagCollection::GetValueType<DscUi::UiRenderTarget*>(in_input_array[6]);
                const DscCommon::VectorInt2 input_texture_viewport_size = input_texture->GetViewportSize();
                buffer._texture_param_0[0] = static_cast<float>(input_texture_viewport_size.GetX());
                buffer._texture_param_0[1] = static_cast<float>(input_texture_viewport_size.GetY());
                const DscCommon::VectorInt2 input_texture_size = input_texture->GetTextureSize();
                buffer._texture_param_0[2] = static_cast<float>(input_texture_size.GetX());
                buffer._texture_param_0[3] = static_cast<float>(input_texture_size.GetY());
                shader->SetShaderResourceViewHandle(0, input_texture->GetTexture());
            }
            if (1 < in_input_texture_count)
            {
                DscUi::UiRenderTarget* const input_texture = DscDag::DagCollection::GetValueType<DscUi::UiRenderTarget*>(in_input_array[7]);
                const DscCommon::VectorInt2 input_texture_viewport_size = input_texture->GetViewportSize();
                buffer._texture_param_0[0] = static_cast<float>(input_texture_viewport_size.GetX());
                buffer._texture_param_0[1] = static_cast<float>(input_texture_viewport_size.GetY());
                const DscCommon::VectorInt2 input_texture_size = input_texture->GetTextureSize();
                buffer._texture_param_0[2] = static_cast<float>(input_texture_size.GetX());
                buffer._texture_param_0[3] = static_cast<float>(input_texture_size.GetY());
                shader->SetShaderResourceViewHandle(1, input_texture->GetTexture());
            }

            if (true == ui_render_target->ActivateRenderTarget(*frame))
            {
                frame->SetShader(shader, shader_buffer);
                frame->Draw(weak_geometry.lock());
                frame->SetRenderTarget(nullptr);
            }
            out_value = ui_render_target.get();
        },
        &in_component_resource_group
    DSC_DEBUG_ONLY(DSC_COMMA in_debug_name + "Draw"));

    auto shader_buffer = in_shader->MakeShaderConstantBuffer(&in_draw_system);
    auto shader_buffer_node = in_dag_collection.CreateValue(
        shader_buffer,
        DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
        &in_component_resource_group
        DSC_DEBUG_ONLY(DSC_COMMA "shader constant"));

    DscDag::DagCollection::LinkIndexNodes(0, in_frame_node, result_node);
    DscDag::DagCollection::LinkIndexNodes(1, in_ui_render_target_node, result_node);
    DscDag::DagCollection::LinkIndexNodes(2, shader_buffer_node, result_node);
    DscDag::DagCollection::LinkIndexNodes(3, in_ui_scale, result_node);
    DscDag::DagCollection::LinkIndexNodes(4, in_effect_param, result_node);
    DscDag::DagCollection::LinkIndexNodes(5, in_effect_tint, result_node);
    for (int32 index = 0; index < in_input_texture_count; ++index)
    {
        DSC_ASSERT(0 < in_array_input_stack.size(), "invalid state");
        DscDag::DagCollection::LinkIndexNodes(6 + index, in_array_input_stack[in_array_input_stack.size() - 1 - index], result_node);
    }

    return result_node;
}

DscDag::NodeToken DscUi::MakeNode::MakeAvaliableSize(
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_parent_avaliable_size,
    DscDag::NodeToken in_ui_scale,
    const DscUi::UiComponentResourceNodeGroup& in_component_resource_group,
    const DscUi::UiComponentResourceNodeGroup& in_parent_component_resource_group,
    DscUi::UiNodeGroup& in_owner_group
)
{
    DscDag::NodeToken node = in_parent_avaliable_size;
    if (nullptr != in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TFlow)) // slot child
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const DscCommon::VectorInt2& parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
                const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[1]);
                const DscUi::TUiFlow flow = DscDag::DagCollection::GetValueType<DscUi::TUiFlow>(in_input_array[2]);
                DscCommon::VectorInt2 result = {};

                switch (flow)
                {
                default:
                    DSC_ASSERT_ALWAYS("invalid switch case");
                    break;
                case DscUi::TUiFlow::THorizontal:
                {
                    if (nullptr != in_input_array[3])
                    {
                        const DscUi::UiCoord& child_size = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[3]);
                        result[1] = child_size.Evaluate(parent_avaliable_size.GetY(), parent_avaliable_size.GetX(), ui_scale);
                    }
                    else
                    {
                        result[1] = parent_avaliable_size.GetY();
                    }
                    if (nullptr != in_input_array[4])
                    {
                        const DscUi::VectorUiCoord2& desired_size_coord = DscDag::DagCollection::GetValueType<DscUi::VectorUiCoord2>(in_input_array[4]);
                        result[0] = desired_size_coord.EvalueUICoord(parent_avaliable_size, ui_scale).GetX();
                    }
                }
                break;
                case DscUi::TUiFlow::TVertical:
                {
                    if (nullptr != in_input_array[3])
                    {
                        const DscUi::UiCoord& child_size = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[3]);
                        result[0] = child_size.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale);
                    }
                    else
                    {
                        result[0] = parent_avaliable_size.GetX();
                    }
                    if (nullptr != in_input_array[4])
                    {
                        const DscUi::VectorUiCoord2& desired_size_coord = DscDag::DagCollection::GetValueType<DscUi::VectorUiCoord2>(in_input_array[4]);
                        result[1] = desired_size_coord.EvalueUICoord(parent_avaliable_size, ui_scale).GetY();
                    }
                }
                }

                value = result;
            },
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "avaliable size child stack"));

        DscDag::DagCollection::LinkIndexNodes(0, in_parent_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_scale, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TFlow), node);
        DscDag::DagCollection::LinkIndexNodes(3, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildStackSize), node);
        DscDag::DagCollection::LinkIndexNodes(4, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TDesiredSize), node);

    }
    else if (nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildSlotSize)) // canvas child
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const DscCommon::VectorInt2& parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
                const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[1]);
                const DscUi::VectorUiCoord2& child_slot_size = DscDag::DagCollection::GetValueType<DscUi::VectorUiCoord2>(in_input_array[2]);

                value = child_slot_size.EvalueUICoord(parent_avaliable_size, ui_scale);
            },
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "avaliable size child slot"));

        DscDag::DagCollection::LinkIndexNodes(0, in_parent_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_scale, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildSlotSize), node);
    }
    else if (nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingLeft)) // padding child
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const DscCommon::VectorInt2& parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
                const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[1]);
                const DscUi::UiCoord& left = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[2]);
                const DscUi::UiCoord& top = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[3]);
                const DscUi::UiCoord& right = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[4]);
                const DscUi::UiCoord& bottom = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[5]);

                const int32 width_taken_by_padding = left.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale) +
                    right.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale);
                const int32 height_taken_by_padding = top.Evaluate(parent_avaliable_size.GetY(), parent_avaliable_size.GetX(), ui_scale) +
                    bottom.Evaluate(parent_avaliable_size.GetY(), parent_avaliable_size.GetX(), ui_scale);

                value.Set(
                    parent_avaliable_size.GetX() - width_taken_by_padding,
                    parent_avaliable_size.GetY() - height_taken_by_padding
                );
            },
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "avaliable size padding"));

        DscDag::DagCollection::LinkIndexNodes(0, in_parent_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_scale, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingLeft), node);
        DscDag::DagCollection::LinkIndexNodes(3, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingTop), node);
        DscDag::DagCollection::LinkIndexNodes(4, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingRight), node);
        DscDag::DagCollection::LinkIndexNodes(5, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingBottom), node);
    }
    else if (nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TDesiredSize))
    {
        // so, flow set one axis of a avaliable size to zero
        // we can't have child layout use desired size, as that can shring to children, but we can inflate the avalaible size of nodes with desired size...
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscUi::VectorUiCoord2& desired_size_coord = DscDag::DagCollection::GetValueType<DscUi::VectorUiCoord2>(in_input_array[0]);
            const DscCommon::VectorInt2& parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[2]);

            // desired size is usually based on the node avalaible, not the parent, but the default case for avaliable is the parent
            const DscCommon::VectorInt2 desired_size = desired_size_coord.EvalueUICoord(parent_avaliable_size, ui_scale);
            value.Set(
                std::max(desired_size.GetX(), parent_avaliable_size.GetX()),
                std::max(desired_size.GetY(), parent_avaliable_size.GetY())
                );
        },
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "avaliable size desired size"));

        DscDag::DagCollection::LinkIndexNodes(0, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TDesiredSize), node);
        DscDag::DagCollection::LinkIndexNodes(1, in_parent_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_ui_scale, node);

    }
    return node;
}

DscDag::NodeToken DscUi::MakeNode::MakeDesiredSize(
    const DscUi::TUiComponentType in_component_type,
    const bool in_desired_size_from_children_max,
    DscDag::DagCollection & in_dag_collection,
    DscDag::NodeToken in_ui_scale,
    DscDag::NodeToken in_avaliable_size,
    DscDag::NodeToken in_array_child_node_group,
    const DscUi::UiComponentResourceNodeGroup& in_resource_node_group,
    DscUi::UiNodeGroup & in_owner_group
)
{
    DSC_UNUSED(in_component_type);
    DSC_UNUSED(in_resource_node_group);
    DSC_UNUSED(in_ui_scale);
    DscDag::NodeToken node = in_avaliable_size;

    if (true == in_desired_size_from_children_max)
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const std::vector<DscUi::UiNodeGroup>& array_child_node_group = DscDag::DagCollection::GetValueType<std::vector<DscUi::UiNodeGroup>>(in_input_array[0]);
                DscCommon::VectorInt2 max_size = {};
                for (const auto& item : array_child_node_group)
                {
                    const DscCommon::VectorInt2& geometry_offset = DscDag::DagCollection::GetValueType< DscCommon::VectorInt2>(item.GetNodeToken(DscUi::TUiNodeGroup::TGeometryOffset));
                    const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType< DscCommon::VectorInt2>(item.GetNodeToken(DscUi::TUiNodeGroup::TGeometrySize));

                    max_size.Set(
                        std::max(max_size.GetX(), geometry_offset.GetX() + geometry_size.GetX()),
                        std::max(max_size.GetY(), geometry_offset.GetY() + geometry_size.GetY())
                    );
                }

                value = max_size;
            },
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "desired size max children"));

        DscDag::DagCollection::LinkIndexNodes(0, in_array_child_node_group, node);
    }
    else if (nullptr != in_resource_node_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TDesiredSize))
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const DscUi::VectorUiCoord2& desired_size_coord = DscDag::DagCollection::GetValueType<DscUi::VectorUiCoord2>(in_input_array[0]);
                const DscCommon::VectorInt2& avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
                const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[2]);

                value = desired_size_coord.EvalueUICoord(avaliable_size, ui_scale);
            },
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "desired size resource group"));

        DscDag::DagCollection::LinkIndexNodes(0, in_resource_node_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TDesiredSize), node);
        DscDag::DagCollection::LinkIndexNodes(1, in_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_ui_scale, node);
        
    }
    else if (nullptr != in_resource_node_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TText))        //if text, get text bounds size
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const DscCommon::VectorInt2& avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
                const DscUi::TUiComponentTextData& text_data = DscDag::DagCollection::GetValueType<DscUi::TUiComponentTextData>(in_input_array[1]);
                const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[2]);

                DscText::TextRun* const text_run_raw = text_data._text_run.get();
                if (nullptr != text_run_raw)
                {
                    text_run_raw->SetWidthLimit(
                        text_run_raw->GetWidthLimitEnabled(),
                        avaliable_size.GetX()
                    );
                    text_run_raw->SetUIScale(ui_scale);

                    value = text_run_raw->GetTextBounds();
                }
            },
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "desired size text"));

        DscDag::DagCollection::LinkIndexNodes(0, in_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_resource_node_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TText), node);
        DscDag::DagCollection::LinkIndexNodes(2, in_resource_node_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TUiScale), node);
    }

    return node;
}

DscDag::NodeToken DscUi::MakeNode::MakeRenderRequestSize(
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_desired_size,
    DscDag::NodeToken in_geometry_size_size,
    DscUi::UiNodeGroup& in_owner_group
    )
{
    DscDag::NodeToken node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscCommon::VectorInt2& desired_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

            value.Set(
                std::max(desired_size.GetX(), geometry_size.GetX()),
                std::max(desired_size.GetY(), geometry_size.GetY())
                );
        },
        &in_owner_group
        DSC_DEBUG_ONLY(DSC_COMMA "render request size"));

    DscDag::DagCollection::LinkIndexNodes(0, in_desired_size, node);
    DscDag::DagCollection::LinkIndexNodes(1, in_geometry_size_size, node);
    return node;
}

DscDag::NodeToken DscUi::MakeNode::MakeGeometrySize(
    DscDag::DagCollection& in_dag_collection,
    const DscUi::UiComponentResourceNodeGroup& in_component_resource_group,
    const DscUi::UiComponentResourceNodeGroup& in_parent_component_resource_group,
    DscDag::NodeToken in_desired_size,
    DscDag::NodeToken in_avaliable_size,
    DscUi::UiNodeGroup& in_owner_group
)
{
    DSC_UNUSED(in_dag_collection);
    DSC_UNUSED(in_owner_group);
    DscDag::NodeToken node = in_desired_size;
    if (nullptr != in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TFlow))
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const DscUi::TUiFlow flow = DscDag::DagCollection::GetValueType<DscUi::TUiFlow>(in_input_array[0]);
                const DscCommon::VectorInt2& desired_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
                const DscCommon::VectorInt2& avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[2]);

                switch (flow)
                {
                default:
                    value = {}; //.Set(0, 0);
                    DSC_ASSERT_ALWAYS("invalid switch case");
                    break;
                case DscUi::TUiFlow::THorizontal:
                    value.Set(
                        desired_size.GetX(),
                        avaliable_size.GetY()
                    );
                case DscUi::TUiFlow::TVertical:
                    value.Set(
                        avaliable_size.GetX(),
                        desired_size.GetY()
                    );
                }
            },
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "geometry size flow child"));

        DscDag::DagCollection::LinkIndexNodes(0, in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TFlow), node);
        DscDag::DagCollection::LinkIndexNodes(1, in_desired_size, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_avaliable_size, node);

    } 
    // do we need to reduce the geoemtry size for the padding?
    else if (nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildSlotSize))
        //((nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildSlotSize)) ||
        //(nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingLeft)))
    {
        // if we have a child slot, presume we are behaving like a canvas that has the geometry size the same as the avaliable size
        node = in_avaliable_size;
    }

    else

    DSC_ASSERT(nullptr != node, "invalid state");
    return node;
}

DscDag::NodeToken DscUi::MakeNode::MakeGeometryOffset(
    DscDag::DagCollection& in_dag_collection,
    const DscUi::UiComponentResourceNodeGroup& in_component_resource_group,
    const DscUi::UiComponentResourceNodeGroup& in_parent_component_resource_group,
    DscDag::NodeToken in_parent_avaliable_size,
    DscDag::NodeToken in_ui_scale,
    DscDag::NodeToken in_geometry_size,
    DscDag::NodeToken in_parent_array_child_ui_node_group,
    DscUi::UiNodeGroup& in_owner_group
)
{
    DSC_UNUSED(in_parent_array_child_ui_node_group);

    DscDag::NodeToken node = nullptr;
    if (nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildSlotSize))
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const DscCommon::VectorInt2& parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
                const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[1]);
                const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[2]);

                const DscUi::VectorUiCoord2& child_slot_pivot = DscDag::DagCollection::GetValueType<DscUi::VectorUiCoord2>(in_input_array[3]);
                const DscUi::VectorUiCoord2& child_slot_parent_attach = DscDag::DagCollection::GetValueType<DscUi::VectorUiCoord2>(in_input_array[4]);

                const DscCommon::VectorInt2 pivot_point = child_slot_pivot.EvalueUICoord(geometry_size, ui_scale);
                const DscCommon::VectorInt2 attach_point = child_slot_parent_attach.EvalueUICoord(parent_avaliable_size, ui_scale);

                value.Set(
                    attach_point.GetX() - pivot_point.GetX(),
                    attach_point.GetY() - pivot_point.GetY()
                );
            },
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "geometry offset child slot"));

        DscDag::DagCollection::LinkIndexNodes(0, in_parent_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_scale, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_geometry_size, node); // should this be avaliable size or geometry size
        DscDag::DagCollection::LinkIndexNodes(3, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildSlotPivot), node);
        DscDag::DagCollection::LinkIndexNodes(4, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildSlotParentAttach), node);
    }
    else if (nullptr != in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TFlow))
    {
        // this is a bit bad, we don't have an easy way of knowing what array child index we are of our parent (in a fashion resiliant to array size change) 
        // we could use the geoemtry size node as a token to identify ourself since we have that node on hand...
        //TUiNodeGroup::TGeometrySize
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
                const DscDag::NodeToken self_geometry_size = in_input_array[0];
                const std::vector<DscUi::UiNodeGroup>& parent_child_array = DscDag::DagCollection::GetValueType<std::vector<DscUi::UiNodeGroup>>(in_input_array[1]);
                const DscUi::UiCoord& gap = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[2]);
                const DscCommon::VectorInt2& parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[3]);
                const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[4]);
                const DscUi::TUiFlow flow = DscDag::DagCollection::GetValueType<DscUi::TUiFlow>(in_input_array[5]);

                value = {};
                int32 trace = 0;
                for (const auto& item : parent_child_array)
                {
                    if (self_geometry_size == item.GetNodeToken(DscUi::TUiNodeGroup::TGeometrySize))
                    {
                        break;
                    }
                    const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(item.GetNodeToken(DscUi::TUiNodeGroup::TGeometrySize));
                    switch (flow)
                    {
                    default:
                        break;
                    case DscUi::TUiFlow::THorizontal:
                        trace += geometry_size.GetX();
                        trace += gap.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale);
                        break;
                    case DscUi::TUiFlow::TVertical:
                        trace += geometry_size.GetY();
                        trace += gap.Evaluate(parent_avaliable_size.GetY(), parent_avaliable_size.GetX(), ui_scale);
                        break;
                    }
                }

                const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(self_geometry_size);
                switch (flow)
                {
                default:
                    break;
                case DscUi::TUiFlow::THorizontal:
                {
                    int32 offset = 0;
                    if (7 < in_input_array.size())
                    { 
                        const DscUi::UiCoord& pivot = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[6]);
                        const DscUi::UiCoord& parent_attach = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[7]);
                        offset = parent_attach.Evaluate(parent_avaliable_size.GetY(), parent_avaliable_size.GetX(), ui_scale) -
                            pivot.Evaluate(geometry_size.GetY(), geometry_size.GetX(), ui_scale);
                    }
                    value.Set(trace, offset);
                }
                break;
                case DscUi::TUiFlow::TVertical:
                {
                    int32 offset = 0;
                    if (7 < in_input_array.size())
                    {
                        const DscUi::UiCoord& pivot = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[6]);
                        const DscUi::UiCoord& parent_attach = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[7]);
                        offset = parent_attach.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale) -
                            pivot.Evaluate(geometry_size.GetX(), geometry_size.GetY(), ui_scale);
                    }
                    value.Set(offset, trace);
                }
                break;
                }

                return;
            },
            & in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "geometry offset child stack"));
        DscDag::DagCollection::LinkIndexNodes(0, in_geometry_size, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_parent_array_child_ui_node_group, node);
        DSC_ASSERT(nullptr != in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TGap), "parent stack is obliged to have a gap node");
        DscDag::DagCollection::LinkIndexNodes(2, in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TGap), node);
        DscDag::DagCollection::LinkIndexNodes(3, in_parent_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(4, in_ui_scale, node);
        DscDag::DagCollection::LinkIndexNodes(5, in_parent_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TFlow), node);
        if ((nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildStackPivot)) &&
            (nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildStackParentAttach)))
        {
            DscDag::DagCollection::LinkIndexNodes(6, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildStackPivot), node);
            DscDag::DagCollection::LinkIndexNodes(7, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildStackParentAttach), node);
        }
    }
    else if ((nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingLeft)) &&
        (nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingTop)))
    {
        // using Parent avaliable size to match what is done in the other usage of padding
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscCommon::VectorInt2& parent_avaliable_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            const float ui_scale = DscDag::DagCollection::GetValueType<float>(in_input_array[1]);
            const DscUi::UiCoord& padding_left = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[2]);
            const DscUi::UiCoord& padding_top = DscDag::DagCollection::GetValueType<DscUi::UiCoord>(in_input_array[3]);

            value.Set(
                padding_left.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale),
                padding_top.Evaluate(parent_avaliable_size.GetY(), parent_avaliable_size.GetX(), ui_scale)
            );
        },
        &in_owner_group
        DSC_DEBUG_ONLY(DSC_COMMA "geometry offset padding"));

        DscDag::DagCollection::LinkIndexNodes(0, in_parent_avaliable_size, node);
        DscDag::DagCollection::LinkIndexNodes(1, in_ui_scale, node);
        DscDag::DagCollection::LinkIndexNodes(2, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingLeft), node);
        DscDag::DagCollection::LinkIndexNodes(3, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingTop), node);
    }
    else
    {
        node = in_dag_collection.CreateValue<DscCommon::VectorInt2>(
            DscCommon::VectorInt2::s_zero,
            DscDag::CallbackOnValueChange<DscCommon::VectorInt2>::Function,
            &in_owner_group
            DSC_DEBUG_ONLY(DSC_COMMA "geometry offset zero"));
    }

    return node;
}

DscDag::NodeToken DscUi::MakeNode::MakeNodePixelTraversal(
    DscDag::DagCollection& in_dag_collection, 
    DscDag::NodeToken in_geometry_size, 
    DscDag::NodeToken in_render_request_size,
    DscUi::UiNodeGroup& in_owner_group
    )
{
    DscDag::NodeToken node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            const DscCommon::VectorInt2& render_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

            value.Set(
                std::max(0, render_size.GetX() - geometry_size.GetX()),
                std::max(0, render_size.GetY() - geometry_size.GetY())
            );
        },
        &in_owner_group
        DSC_DEBUG_ONLY(DSC_COMMA "pixel traversal"));

    DscDag::DagCollection::LinkIndexNodes(0, in_geometry_size, node);
    DscDag::DagCollection::LinkIndexNodes(1, in_render_request_size, node);

    return node;
}

/// note: the automatic scroll, to do pingpoing without state, traverses a range of [-1 ... 1] and is passed through an abs() function
/// otherwise would need state of going up or going down and swap at [0, 1]
/// this works, but manual scroll needs to clamp it's range [0 ... 1] else in negative domain, will be pingpong to positive....
DscDag::NodeToken DscUi::MakeNode::MakeNodeScrollValue(
    DscDag::DagCollection& in_dag_collection,
    const DscUi::UiComponentResourceNodeGroup& in_component_resource_group,
    DscDag::NodeToken in_time_delta,
    DscDag::NodeToken in_pixel_traversal_node,
    DscUi::UiNodeGroup& in_owner_group
)
{
    DscDag::NodeToken condition_x = in_dag_collection.CreateCalculate<bool>([](bool& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            // if manual scroll is false and the 0 < pixel traversal, return true
            const bool has_manual_scroll = DscDag::DagCollection::GetValueType<bool>(in_input_array[0]);
            const DscCommon::VectorInt2& pixel_traversal = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

            out_value = ((false == has_manual_scroll) && (0 < pixel_traversal.GetX()));
        },
        &in_owner_group
        DSC_DEBUG_ONLY(DSC_COMMA "scroll condition x"));
    DscDag::DagCollection::LinkIndexNodes(0, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::THasManualScrollX), condition_x);
    DscDag::DagCollection::LinkIndexNodes(1, in_pixel_traversal_node, condition_x);

    DscDag::NodeToken condition_y = in_dag_collection.CreateCalculate<bool>([](bool& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            // if manual scroll is false and the 0 < pixel traversal, return true
            const bool has_manual_scroll = DscDag::DagCollection::GetValueType<bool>(in_input_array[0]);
            const DscCommon::VectorInt2& pixel_traversal = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

            out_value = ((false == has_manual_scroll) && (0 < pixel_traversal.GetY()));
        },
        & in_owner_group
        DSC_DEBUG_ONLY(DSC_COMMA "scroll condition y"));
    DscDag::DagCollection::LinkIndexNodes(0, in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::THasManualScrollY), condition_y);
    DscDag::DagCollection::LinkIndexNodes(1, in_pixel_traversal_node, condition_y);

    DscDag::NodeToken tick_scroll_x = in_dag_collection.CreateCalculate<float>([](float& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const float time_delta = DscDag::DagCollection::GetValueType<float>(in_input_array[0]);
            const float time_delta_clamped = std::max(0.0f, std::min(1.0f, time_delta));
            const int32 pixel_traversal = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]).GetX();

            if (0 < pixel_traversal)
            {
                // the max of the divisor is to avoid things bouncing too quickly on small values of pixel_traversal
                out_value += time_delta_clamped * s_scroll_pixels_per_second / std::max(s_scroll_pixels_per_second, static_cast<float>(pixel_traversal));
                while (s_wrap_threashold < out_value)
                {
                    out_value -= s_wrap_step_ping_pong; // pingpong, consumer of the value applies std::abs
                }
            }
        },
        &in_owner_group
        DSC_DEBUG_ONLY(DSC_COMMA "scroll condition x"));
    DscDag::DagCollection::LinkIndexNodes(0, in_time_delta, tick_scroll_x);
    DscDag::DagCollection::LinkIndexNodes(1, in_pixel_traversal_node, tick_scroll_x);

    DscDag::NodeToken tick_scroll_y = in_dag_collection.CreateCalculate<float>([](float& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const float time_delta = DscDag::DagCollection::GetValueType<float>(in_input_array[0]);
            const float time_delta_clamped = std::max(0.0f, std::min(1.0f, time_delta));
            const int32 pixel_traversal = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]).GetY();

            if (0 < pixel_traversal)
            {
                // the max of the divisor is to avoid things bouncing too quickly on small values of pixel_traversal
                out_value += time_delta_clamped * s_scroll_pixels_per_second / std::max(s_scroll_pixels_per_second, static_cast<float>(pixel_traversal));
                while (s_wrap_threashold < out_value)
                {
                    out_value -= s_wrap_step_ping_pong; // pingpong, consumer of the value applies std::abs
                }
            }
        },
        & in_owner_group
        DSC_DEBUG_ONLY(DSC_COMMA "scroll condition y"));
    DscDag::DagCollection::LinkIndexNodes(0, in_time_delta, tick_scroll_y);
    DscDag::DagCollection::LinkIndexNodes(1, in_pixel_traversal_node, tick_scroll_y);

    // something to hold the scroll value output
    DscDag::NodeToken scroll_x = in_dag_collection.CreateValue(
        0.0f, 
        DscDag::CallbackOnValueChange<float>::Function,
        &in_owner_group
        DSC_DEBUG_ONLY(DSC_COMMA "scroll x"));
    DscDag::NodeToken scroll_y = in_dag_collection.CreateValue(
        0.0f,
        DscDag::CallbackOnValueChange<float>::Function,
        &in_owner_group
        DSC_DEBUG_ONLY(DSC_COMMA "scroll y"));

    in_dag_collection.CreateCondition(
        condition_x,
        tick_scroll_x, 
        in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TManualScrollX),
        scroll_x, 
        scroll_x,
        &in_owner_group
        DSC_DEBUG_ONLY(DSC_COMMA "conditional scroll x")
    );
    in_dag_collection.CreateCondition(
        condition_y,
        tick_scroll_y,
        in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TManualScrollY),
        scroll_y,
        scroll_y,
        &in_owner_group
        DSC_DEBUG_ONLY(DSC_COMMA "conditional scroll y")
    );

    DscDag::NodeToken result_node = in_dag_collection.CreateCalculate<DscCommon::VectorFloat2>([](DscCommon::VectorFloat2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const float x = DscDag::DagCollection::GetValueType<float>(in_input_array[0]);
            const float y = DscDag::DagCollection::GetValueType<float>(in_input_array[1]);

            value.Set(x, y);
        },
        &in_owner_group
    DSC_DEBUG_ONLY(DSC_COMMA "scroll"));
    DscDag::DagCollection::LinkIndexNodes(0, scroll_x, result_node);
    DscDag::DagCollection::LinkIndexNodes(1, scroll_y, result_node);

    return result_node;
}

DscDag::NodeToken DscUi::MakeNode::MakeScreenSpace(
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_parent_screen_space,
    DscDag::NodeToken in_parent_render_request_size,
    DscDag::NodeToken in_geometry_size,
    DscDag::NodeToken in_geometry_offset,
    DscDag::NodeToken in_render_request_size,
    DscDag::NodeToken in_scroll,
    DscUi::UiNodeGroup& in_owner_group
)
{
    DscDag::NodeToken node = in_dag_collection.CreateCalculate<DscUi::ScreenSpace>([](DscUi::ScreenSpace& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscUi::ScreenSpace& parent_screen_space = DscDag::DagCollection::GetValueType<DscUi::ScreenSpace>(in_input_array[0]);
            const DscCommon::VectorInt2& parent_render_request_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            const DscCommon::VectorInt2& geometry_offset = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[2]);
            const DscCommon::VectorInt2& geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[3]);
            const DscCommon::VectorInt2& render_request_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(in_input_array[4]);
            const DscCommon::VectorFloat2& scroll = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat2>(in_input_array[5]);

            const DscCommon::VectorFloat2 parent_size(
                parent_screen_space._screen_space[2] - parent_screen_space._screen_space[0],
                parent_screen_space._screen_space[3] - parent_screen_space._screen_space[1]
                );
            const DscCommon::VectorFloat2 parent_scale(
                parent_size.GetX() / static_cast<float>(parent_render_request_size.GetX()),
                parent_size.GetY() / static_cast<float>(parent_render_request_size.GetY())
            );

            const DscCommon::VectorFloat2 geometry_pivot(
                static_cast<float>(geometry_offset.GetX()) - (static_cast<float>(render_request_size.GetX() - geometry_size.GetX()) * std::abs(scroll.GetX())),
                static_cast<float>(geometry_offset.GetY()) - (static_cast<float>(render_request_size.GetY() - geometry_size.GetY()) * std::abs(scroll.GetY()))
                );

            value._screen_space.Set(
                parent_screen_space._screen_space[0] + (geometry_pivot[0] * parent_scale[0]),
                parent_screen_space._screen_space[1] + (geometry_pivot[1] * parent_scale[1]),
                parent_screen_space._screen_space[0] + ((geometry_pivot[0] + static_cast<float>(render_request_size.GetX())) * parent_scale[0]),
                parent_screen_space._screen_space[1] + ((geometry_pivot[1] + static_cast<float>(render_request_size.GetY())) * parent_scale[1])
                );

            const DscCommon::VectorFloat2 geometry_coverage(
                static_cast<float>(geometry_size.GetX()) / static_cast<float>(render_request_size.GetX()),
                static_cast<float>(geometry_size.GetY()) / static_cast<float>(render_request_size.GetY())
                );

            const DscCommon::VectorFloat2 geometry_size_valid(
                value._screen_space[2] - value._screen_space[0],
                value._screen_space[3] - value._screen_space[1]
                );

            const DscCommon::VectorFloat4 geometry_valid(
                value._screen_space[0] + (geometry_size_valid[0] * ((1.0f - geometry_coverage[0]) * std::abs(scroll.GetX()))),
                value._screen_space[1] + (geometry_size_valid[1] * ((1.0f - geometry_coverage[1]) * std::abs(scroll.GetY()))),
                value._screen_space[0] + (geometry_size_valid[0] * (geometry_coverage[0] + ((1.0f - geometry_coverage[0]) * std::abs(scroll.GetX())))),
                value._screen_space[1] + (geometry_size_valid[1] * (geometry_coverage[1] + ((1.0f - geometry_coverage[1]) * std::abs(scroll.GetY()))))
                );

            value._screen_valid.Set(
                std::max(parent_screen_space._screen_valid[0], geometry_valid[0]),
                std::max(parent_screen_space._screen_valid[1], geometry_valid[1]),
                std::min(parent_screen_space._screen_valid[2], geometry_valid[2]),
                std::min(parent_screen_space._screen_valid[3], geometry_valid[3])
            );
        },
        &in_owner_group
        DSC_DEBUG_ONLY(DSC_COMMA "screen space"));

    DscDag::DagCollection::LinkIndexNodes(0, in_parent_screen_space, node);
    DscDag::DagCollection::LinkIndexNodes(1, in_parent_render_request_size, node);
    DscDag::DagCollection::LinkIndexNodes(2, in_geometry_offset, node);
    DscDag::DagCollection::LinkIndexNodes(3, in_geometry_size, node);
    DscDag::DagCollection::LinkIndexNodes(4, in_render_request_size, node);
    DscDag::DagCollection::LinkIndexNodes(5, in_scroll, node);

    return node;
}
