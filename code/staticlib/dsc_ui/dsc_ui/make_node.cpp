#include "make_node.h"

#include "component_construction_helper.h"
#include "screen_quad.h"
#include "ui_enum.h"
#include "ui_render_target.h"
#include "ui_input_param.h"
#include "ui_input_state.h"
#include "ui_manager.h"
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
#include <dsc_text\text.h>


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
    DscDag::IDagOwner* const in_owner
	DSC_DEBUG_ONLY(DSC_COMMA const std::string & in_debug_name)
)
{
    DscDag::NodeToken node = in_dag_collection.CreateCalculate<std::shared_ptr<DscUi::UiRenderTarget>>([
		&in_render_target_pool, &in_draw_system
		DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
		](std::shared_ptr<DscUi::UiRenderTarget>& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscCommon::VectorInt2 request_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            const DscCommon::VectorFloat4 clear_colour = DscDag::GetValueType<DscCommon::VectorFloat4>(in_input_array[1]);

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

			//DSC_DEBUG_ONLY(DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "%s\n", in_debug_name.c_str()));
            value->UpdateRenderTargetPool(
                in_draw_system,
                in_render_target_pool,
                request_size,
                clear_colour
            );
        },
        in_owner
        );
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "ui render target"));

    DscDag::LinkIndexNodes(0, in_request_size_node, node);
    DscDag::LinkIndexNodes(1, in_clear_colour, node);

    return node;
}

DscDag::NodeToken DscUi::MakeNode::MakeEffectDrawNode(
    const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_geometry,
    const std::shared_ptr<DscRenderResource::Shader>& in_shader,
    DscDag::DagCollection& in_dag_collection,
    DscRender::DrawSystem& in_draw_system,
    DscDag::NodeToken in_frame_node,
    DscDag::NodeToken in_ui_render_target_node,
	DscDag::NodeToken in_visible,
    DscDag::NodeToken in_ui_scale,
    DscDag::NodeToken in_effect_strength,
    DscDag::NodeToken in_effect_param,
    DscDag::NodeToken in_effect_tint,
    const std::vector<DscDag::NodeToken>& in_array_input_stack,
    const int32 in_input_texture_count,
    DscDag::IDagOwner* const in_owner
    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name))
{
    DSC_ASSERT(nullptr != in_geometry, "invalid param");
    DSC_ASSERT(nullptr != in_shader, "invalid param");

    std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = in_geometry;
    std::weak_ptr<DscRenderResource::Shader> weak_shader = in_shader;
    DscDag::NodeToken result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>(
	[weak_geometry, weak_shader, in_input_texture_count]// DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)]
	(DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
		//#if defined(_DEBUG)
		//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "Draw node calculate effect:%s\n", in_debug_name.c_str());
		//#endif// defined(_DEBUG)

        DscRenderResource::Frame* const frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
        DSC_ASSERT(nullptr != frame, "invalid state");
        const auto& ui_render_target = DscDag::GetValueType<std::shared_ptr<DscUi::UiRenderTarget>>(in_input_array[1]);
        DSC_ASSERT(nullptr != ui_render_target, "invalid state");
		const bool visible = DscDag::GetValueType<bool>(in_input_array[2]);
		ui_render_target->SetEnabled(visible);
		if (true == visible)
		{
			const auto& shader_buffer = DscDag::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[3]);
			DSC_ASSERT(nullptr != shader_buffer, "invalid state");
			const float ui_scale = DscDag::GetValueType<float>(in_input_array[4]);
			const float effect_strength = (nullptr != in_input_array[5]) ? DscDag::GetValueType<float>(in_input_array[5]) : 1.0f;
			if (0.0f == effect_strength)
			{
				out_value = nullptr;
				if ((0 < in_input_texture_count) && (nullptr != in_input_array[8]))
				{
					out_value = DscDag::GetValueType<DscUi::UiRenderTarget*>(in_input_array[8]);
				}
				return;
			}

			const DscCommon::VectorFloat4& effect_param = DscDag::GetValueType<DscCommon::VectorFloat4>(in_input_array[6]);
			const DscCommon::VectorFloat4& effect_tint = DscDag::GetValueType<DscCommon::VectorFloat4>(in_input_array[7]);

			const DscCommon::VectorInt2 viewport_size = ui_render_target->GetViewportSize();

			auto& buffer = shader_buffer->GetConstant<DscUi::TEffectConstantBuffer>(0);
			buffer._width_height[0] = static_cast<float>(viewport_size.GetX());
			buffer._width_height[1] = static_cast<float>(viewport_size.GetY());
			buffer._width_height[2] = effect_strength;
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
				DscUi::UiRenderTarget* const input_texture = DscDag::GetValueType<DscUi::UiRenderTarget*>(in_input_array[8]);
				//ui_render_target->SetEnabled(input_texture->GetEnabled());
				const DscCommon::VectorInt2 input_texture_viewport_size = input_texture->GetViewportSize();
				buffer._texture_param_0[0] = static_cast<float>(input_texture_viewport_size.GetX());
				buffer._texture_param_0[1] = static_cast<float>(input_texture_viewport_size.GetY());
				const DscCommon::VectorInt2 input_texture_size = input_texture->GetTextureSize();
				buffer._texture_param_0[2] = static_cast<float>(input_texture_size.GetX());
				buffer._texture_param_0[3] = static_cast<float>(input_texture_size.GetY());
				shader->SetShaderResourceViewHandle(0, nullptr);
				shader->SetShaderResourceViewHandle(0, input_texture->GetTexture());
			}
			if (1 < in_input_texture_count) //&& (true == ui_render_target->GetEnabled()))
			{
				DscUi::UiRenderTarget* const input_texture = DscDag::GetValueType<DscUi::UiRenderTarget*>(in_input_array[9]);
				//ui_render_target->SetEnabled(input_texture->GetEnabled());
				const DscCommon::VectorInt2 input_texture_viewport_size = input_texture->GetViewportSize();
				buffer._texture_param_1[0] = static_cast<float>(input_texture_viewport_size.GetX());
				buffer._texture_param_1[1] = static_cast<float>(input_texture_viewport_size.GetY());
				const DscCommon::VectorInt2 input_texture_size = input_texture->GetTextureSize();
				buffer._texture_param_1[2] = static_cast<float>(input_texture_size.GetX());
				buffer._texture_param_1[3] = static_cast<float>(input_texture_size.GetY());
				shader->SetShaderResourceViewHandle(1, nullptr);
				shader->SetShaderResourceViewHandle(1, input_texture->GetTexture());
			}

			if (true == ui_render_target->ActivateRenderTarget(*frame))
			{
				frame->SetShader(shader, shader_buffer);
				frame->Draw(weak_geometry.lock());
				frame->SetRenderTarget(nullptr);
			}
		}

        out_value = ui_render_target.get();
    },
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result_node, in_debug_name));

    auto shader_buffer = in_shader->MakeShaderConstantBuffer(&in_draw_system);
    auto shader_buffer_node = in_dag_collection.CreateValueNone(shader_buffer, in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(shader_buffer_node, "shader constant"));

    DscDag::LinkIndexNodes(0, in_frame_node, result_node);
    DscDag::LinkIndexNodes(1, in_ui_render_target_node, result_node);
    DscDag::LinkIndexNodes(2, in_visible, result_node);
    DscDag::LinkIndexNodes(3, shader_buffer_node, result_node);
    DscDag::LinkIndexNodes(4, in_ui_scale, result_node);
    DscDag::LinkIndexNodes(5, in_effect_strength, result_node);
    DscDag::LinkIndexNodes(6, in_effect_param, result_node);
    DscDag::LinkIndexNodes(7, in_effect_tint, result_node);
    for (int32 index = 0; index < in_input_texture_count; ++index)
    {
        DSC_ASSERT(index < static_cast<int32>(in_array_input_stack.size()), "invalid state");
        DscDag::LinkIndexNodes(8 + index, in_array_input_stack[in_array_input_stack.size() - in_input_texture_count +  index], result_node);
    }

    return result_node;
}

struct TEffectBurnBlotState
{
    int32 _index = 0;
};

DscDag::NodeToken DscUi::MakeNode::MakeEffectBurnBlotDrawNode(
    const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_geometry,
    const std::shared_ptr<DscRenderResource::Shader>& in_shader,
    DscDag::DagCollection& in_dag_collection,
    DscRender::DrawSystem& in_draw_system,
    DscDag::NodeToken in_frame_node,
    DscDag::NodeToken in_ui_render_target_node,
    DscDag::NodeToken in_ui_render_target_node_b,
	DscDag::NodeToken in_visible,
    DscDag::NodeToken in_effect_param,
    DscDag::NodeToken in_effect_tint,
    const std::vector<DscDag::NodeToken>& in_array_input_stack,
    DscDag::IDagOwner* const in_owner
    DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name))
{
    DSC_ASSERT(nullptr != in_geometry, "invalid param");
    DSC_ASSERT(nullptr != in_shader, "invalid param");

    std::weak_ptr<DscRenderResource::GeometryGeneric> weak_geometry = in_geometry;
    std::weak_ptr<DscRenderResource::Shader> weak_shader = in_shader;
    TEffectBurnBlotState state = {};
    DscDag::NodeToken result_node = in_dag_collection.CreateCalculate<DscUi::UiRenderTarget*>([weak_geometry, weak_shader, state](DscUi::UiRenderTarget*& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) mutable {
        DscRenderResource::Frame* const frame = DscDag::GetValueType<DscRenderResource::Frame*>(in_input_array[0]);
        DSC_ASSERT(nullptr != frame, "invalid state");
		const bool visible = DscDag::GetValueType<bool>(in_input_array[1]);
        const auto& ui_render_target_a = DscDag::GetValueType<std::shared_ptr<DscUi::UiRenderTarget>>(in_input_array[2]);
        DSC_ASSERT(nullptr != ui_render_target_a, "invalid state");
        const auto& ui_render_target_b = DscDag::GetValueType<std::shared_ptr<DscUi::UiRenderTarget>>(in_input_array[3]);
        DSC_ASSERT(nullptr != ui_render_target_b, "invalid state");

		ui_render_target_a->SetEnabled(visible);
		ui_render_target_b->SetEnabled(visible);
		DscUi::UiRenderTarget* blot[2];
		blot[0] = ui_render_target_b.get(); 
		blot[1] = ui_render_target_a.get(); 

		if (true == visible)
		{
			const auto& shader_buffer = DscDag::GetValueType<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>(in_input_array[4]);
			DSC_ASSERT(nullptr != shader_buffer, "invalid state");
			const DscCommon::VectorFloat4& effect_param = DscDag::GetValueType<DscCommon::VectorFloat4>(in_input_array[5]);
			const DscCommon::VectorFloat4& effect_tint = DscDag::GetValueType<DscCommon::VectorFloat4>(in_input_array[6]);

			const int32 other_index = state._index;// will be used as blot input
			state._index ^= 1;// will be used as shader output

			const DscCommon::VectorInt2 viewport_size = blot[state._index]->GetViewportSize();

			auto& buffer = shader_buffer->GetConstant<DscUi::TEffectConstantBuffer>(0);
			buffer._width_height[0] = static_cast<float>(viewport_size.GetX());
			buffer._width_height[1] = static_cast<float>(viewport_size.GetY());
			// x. rolled over [0 ... 1] (1 == rolled over), y. time delta [0 ...], zw. mouse pos relative to shader in pixels, uv coords bottom left is 0,0
			buffer._effect_param[0] = effect_param[0];
			buffer._effect_param[1] = effect_param[1];
			buffer._effect_param[2] = effect_param[2];
			buffer._effect_param[3] = effect_param[3];
			buffer._effect_tint[0] = effect_tint[0];
			buffer._effect_tint[1] = effect_tint[1];
			buffer._effect_tint[2] = effect_tint[2];
			buffer._effect_tint[3] = effect_tint[3];

			std::shared_ptr<DscRenderResource::Shader> shader = weak_shader.lock();
			DSC_ASSERT(nullptr != shader, "invalid state");

			// texture zero, the previous draw step
			{
				DscUi::UiRenderTarget* const input_texture = DscDag::GetValueType<DscUi::UiRenderTarget*>(in_input_array[7]);
				//ui_render_target->SetEnabled(input_texture->GetEnabled());
				const DscCommon::VectorInt2 input_texture_viewport_size = input_texture->GetViewportSize();
				buffer._texture_param_0[0] = static_cast<float>(input_texture_viewport_size.GetX());
				buffer._texture_param_0[1] = static_cast<float>(input_texture_viewport_size.GetY());
				const DscCommon::VectorInt2 input_texture_size = input_texture->GetTextureSize();
				buffer._texture_param_0[2] = static_cast<float>(input_texture_size.GetX());
				buffer._texture_param_0[3] = static_cast<float>(input_texture_size.GetY());
				shader->SetShaderResourceViewHandle(0, input_texture->GetTexture());
			}

			// texture one, the previous blot texture
			{
				DscUi::UiRenderTarget* const input_texture_b = blot[other_index];
				//DscUi::UiRenderTarget* const input_texture = DscDag::GetValueType<DscUi::UiRenderTarget*>(in_input_array[6]);
				const DscCommon::VectorInt2 input_texture_viewport_size = input_texture_b->GetViewportSize();
				buffer._texture_param_1[0] = static_cast<float>(input_texture_viewport_size.GetX());
				buffer._texture_param_1[1] = static_cast<float>(input_texture_viewport_size.GetY());
				const DscCommon::VectorInt2 input_texture_size = input_texture_b->GetTextureSize();
				buffer._texture_param_1[2] = static_cast<float>(input_texture_size.GetX());
				buffer._texture_param_1[3] = static_cast<float>(input_texture_size.GetY());
				auto texture = input_texture_b->GetTexture();
				DSC_ASSERT(nullptr != texture, "invalid state");
				shader->SetShaderResourceViewHandle(1, texture);
			}

			if (true == blot[state._index]->ActivateRenderTarget(*frame))
			{
				frame->SetShader(shader, shader_buffer);
				frame->Draw(weak_geometry.lock());
				frame->SetRenderTarget(nullptr);
			}
		}

        out_value = blot[state._index];
    },
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result_node, in_debug_name));

    auto shader_buffer = in_shader->MakeShaderConstantBuffer(&in_draw_system);
    auto shader_buffer_node = in_dag_collection.CreateValue(
        shader_buffer,
        DscDag::CallbackNever<std::shared_ptr<DscRenderResource::ShaderConstantBuffer>>::Function,
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(shader_buffer_node, "shader constant"));

    DscDag::LinkIndexNodes(0, in_frame_node, result_node);
    DscDag::LinkIndexNodes(1, in_visible, result_node);
    DscDag::LinkIndexNodes(2, in_ui_render_target_node, result_node);
    DscDag::LinkIndexNodes(3, in_ui_render_target_node_b, result_node);
    DscDag::LinkIndexNodes(4, shader_buffer_node, result_node);
    DscDag::LinkIndexNodes(5, in_effect_param, result_node);
    DscDag::LinkIndexNodes(6, in_effect_tint, result_node);

    DSC_ASSERT(0 < in_array_input_stack.size(), "invalid state");
    DscDag::LinkIndexNodes(7, in_array_input_stack[in_array_input_stack.size() - 1], result_node);

    return result_node;
}

DscDag::NodeToken DscUi::MakeNode::MakeAvaliableSize(
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_parent_avaliable_size,
    DscDag::NodeToken in_ui_scale,
    DscDag::NodeToken in_component_resource_group,
    DscDag::NodeToken in_parent_component_resource_group,
    DscDag::IDagOwner* const in_owner
)
{
    DscDag::NodeToken node = in_parent_avaliable_size;
    if (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TFlow)) // slot child
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscCommon::VectorInt2& parent_avaliable_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            const float ui_scale = DscDag::GetValueType<float>(in_input_array[1]);
            const DscUi::TUiFlow flow = DscDag::GetValueType<DscUi::TUiFlow>(in_input_array[2]);
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
                    const DscUi::UiCoord& child_size = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[3]);
                    result[1] = child_size.Evaluate(parent_avaliable_size.GetY(), parent_avaliable_size.GetX(), ui_scale);
                }
                else
                {
                    result[1] = parent_avaliable_size.GetY();
                }
                if (nullptr != in_input_array[4])
                {
                    const DscUi::VectorUiCoord2& desired_size_coord = DscDag::GetValueType<DscUi::VectorUiCoord2>(in_input_array[4]);
                    result[0] = desired_size_coord.EvalueUICoord(parent_avaliable_size, ui_scale).GetX();
                }
            }
            break;
            case DscUi::TUiFlow::TVertical:
            {
                if (nullptr != in_input_array[3])
                {
                    const DscUi::UiCoord& child_size = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[3]);
                    result[0] = child_size.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale);
                }
                else
                {
                    result[0] = parent_avaliable_size.GetX();
                }
                if (nullptr != in_input_array[4])
                {
                    const DscUi::VectorUiCoord2& desired_size_coord = DscDag::GetValueType<DscUi::VectorUiCoord2>(in_input_array[4]);
                    result[1] = desired_size_coord.EvalueUICoord(parent_avaliable_size, ui_scale).GetY();
                }
            }
            }

            value = result;
        },
            in_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "avaliable size child stack"));

        DscDag::LinkIndexNodes(0, in_parent_avaliable_size, node);
        DscDag::LinkIndexNodes(1, in_ui_scale, node);
        DscDag::LinkIndexNodes(2, DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TFlow), node);
        DscDag::LinkIndexNodes(3, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TChildStackSize), node);
        DscDag::LinkIndexNodes(4, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TDesiredSize), node);

    }
    else if (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TChildSlotSize)) // canvas child
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscCommon::VectorInt2& parent_avaliable_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            const float ui_scale = DscDag::GetValueType<float>(in_input_array[1]);
            const DscUi::VectorUiCoord2& child_slot_size = DscDag::GetValueType<DscUi::VectorUiCoord2>(in_input_array[2]);

            value = child_slot_size.EvalueUICoord(parent_avaliable_size, ui_scale);
        },
            in_owner);
#if defined(_DEBUG)
        static int s_trace = 0;
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, std::string("avaliable size child slot ") + std::to_string(s_trace++)));
#endif

        DscDag::LinkIndexNodes(0, in_parent_avaliable_size, node);
        DscDag::LinkIndexNodes(1, in_ui_scale, node);
        DscDag::LinkIndexNodes(2, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TChildSlotSize), node);
    }
    else if (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TPaddingLeft)) // padding child
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscCommon::VectorInt2& parent_avaliable_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            const float ui_scale = DscDag::GetValueType<float>(in_input_array[1]);
            const DscUi::UiCoord& left = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[2]);
            const DscUi::UiCoord& top = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[3]);
            const DscUi::UiCoord& right = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[4]);
            const DscUi::UiCoord& bottom = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[5]);

            const int32 width_taken_by_padding = left.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale) +
                right.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale);
            const int32 height_taken_by_padding = top.Evaluate(parent_avaliable_size.GetY(), parent_avaliable_size.GetX(), ui_scale) +
                bottom.Evaluate(parent_avaliable_size.GetY(), parent_avaliable_size.GetX(), ui_scale);

            value.Set(
                parent_avaliable_size.GetX() - width_taken_by_padding,
                parent_avaliable_size.GetY() - height_taken_by_padding
            );
        },
            in_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "avaliable size padding"));

        DscDag::LinkIndexNodes(0, in_parent_avaliable_size, node);
        DscDag::LinkIndexNodes(1, in_ui_scale, node);
        DscDag::LinkIndexNodes(2, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TPaddingLeft), node);
        DscDag::LinkIndexNodes(3, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TPaddingTop), node);
        DscDag::LinkIndexNodes(4, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TPaddingRight), node);
        DscDag::LinkIndexNodes(5, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TPaddingBottom), node);
    }
    else if (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TDesiredSize))
    {
        // so, flow set one axis of a avaliable size to zero
        // we can't have child layout use desired size, as that can shring to children, but we can inflate the avalaible size of nodes with desired size...
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscUi::VectorUiCoord2& desired_size_coord = DscDag::GetValueType<DscUi::VectorUiCoord2>(in_input_array[0]);
            const DscCommon::VectorInt2& parent_avaliable_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            const float ui_scale = DscDag::GetValueType<float>(in_input_array[2]);

            // desired size is usually based on the node avalaible, not the parent, but the default case for avaliable is the parent
            const DscCommon::VectorInt2 desired_size = desired_size_coord.EvalueUICoord(parent_avaliable_size, ui_scale);
            value.Set(
                std::max(desired_size.GetX(), parent_avaliable_size.GetX()),
                std::max(desired_size.GetY(), parent_avaliable_size.GetY())
            );
        },
            in_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "avaliable size desired"));

        DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TDesiredSize), node);
        DscDag::LinkIndexNodes(1, in_parent_avaliable_size, node);
        DscDag::LinkIndexNodes(2, in_ui_scale, node);

    }
    return node;
}

DscDag::NodeToken DscUi::MakeNode::MakeDesiredSize(
    const DscUi::TUiComponentType in_component_type,
    const bool in_desired_size_from_children_max,
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_ui_scale,
    DscDag::NodeToken in_avaliable_size,
    DscDag::NodeToken in_array_child_node_group,
    DscDag::NodeToken in_resource_node_group,
    DscDag::IDagOwner* const in_owner
)
{
    DSC_UNUSED(in_component_type);
    DSC_UNUSED(in_resource_node_group);
    DSC_UNUSED(in_ui_scale);
    DscDag::NodeToken node = in_avaliable_size;

    if (true == in_desired_size_from_children_max)
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            //DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "MakeDesiredSize\n");

            const std::vector<DscDag::NodeToken>& array_child_node_group = DscDag::GetValueNodeArray(in_input_array[0]);
            DscCommon::VectorInt2 max_size = {};
            for (const auto& item : array_child_node_group)
            {
                const DscCommon::VectorInt2& geometry_offset = DscDag::GetValueType< DscCommon::VectorInt2>(DscDag::DagNodeGroup::GetNodeTokenEnum(item, DscUi::TUiNodeGroup::TGeometryOffset));
                const DscCommon::VectorInt2& geometry_size = DscDag::GetValueType< DscCommon::VectorInt2>(DscDag::DagNodeGroup::GetNodeTokenEnum(item, DscUi::TUiNodeGroup::TGeometrySize));

                //DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "  geometry_size[%d, %d] geometry_offset[%d, %d]\n", geometry_size.GetX(), geometry_size.GetY(), geometry_offset.GetX(), geometry_offset.GetY());

                max_size.Set(
                    std::max(max_size.GetX(), geometry_offset.GetX() + geometry_size.GetX()),
                    std::max(max_size.GetY(), geometry_offset.GetY() + geometry_size.GetY())
                );
            }

            //DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, " max_size[%d, %d]\n", max_size.GetX(), max_size.GetY());

            value = max_size;
        },
            in_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "desired size max children"));

        DscDag::LinkIndexNodes(0, in_array_child_node_group, node);

        //in_array_child_node_group is currently not dirtied on resize (node group not set to dirty), so desired size was not recalculating
        DscDag::LinkNodes(in_avaliable_size, node);
    }
    else if (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_resource_node_group, DscUi::TUiComponentResourceNodeGroup::TDesiredSize))
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscUi::VectorUiCoord2& desired_size_coord = DscDag::GetValueType<DscUi::VectorUiCoord2>(in_input_array[0]);
            const DscCommon::VectorInt2& avaliable_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            const float ui_scale = DscDag::GetValueType<float>(in_input_array[2]);

            value = desired_size_coord.EvalueUICoord(avaliable_size, ui_scale);
        },
            in_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "desired size resource group"));

        DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(in_resource_node_group, DscUi::TUiComponentResourceNodeGroup::TDesiredSize), node);
        DscDag::LinkIndexNodes(1, in_avaliable_size, node);
        DscDag::LinkIndexNodes(2, in_ui_scale, node);
        
    }
    else if (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_resource_node_group, DscUi::TUiComponentResourceNodeGroup::TText))        //if text, get text bounds size
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscCommon::VectorInt2& avaliable_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            const DscUi::TUiComponentTextData& text_data = DscDag::GetValueType<DscUi::TUiComponentTextData>(in_input_array[1]);
            const float ui_scale = DscDag::GetValueType<float>(in_input_array[2]);

            DscText::Text* const text_run_raw = text_data._text_run.get();
            if (nullptr != text_run_raw)
            {
                text_run_raw->SetWidthLimit(
                    text_run_raw->GetWidthLimitEnabled(),
                    avaliable_size.GetX()
                );
                text_run_raw->SetUIScale(ui_scale);

                value = text_run_raw->GetTextBounds();
            }

            // inflate desired size by padding
            if ((nullptr != in_input_array[3]) &&
                (nullptr != in_input_array[4]) &&
                (nullptr != in_input_array[5]) &&
                (nullptr != in_input_array[6])
                )
            {
                const DscUi::UiCoord& padding_left = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[3]);
                const DscUi::UiCoord& padding_top = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[4]);
                const DscUi::UiCoord& padding_right = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[5]);
                const DscUi::UiCoord& padding_bottom = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[6]);

                value.Set(
                    value.GetX() + padding_left.Evaluate(avaliable_size.GetX(), avaliable_size.GetY(), ui_scale)
                    + padding_right.Evaluate(avaliable_size.GetX(), avaliable_size.GetY(), ui_scale),
                    value.GetY() + padding_top.Evaluate(avaliable_size.GetY(), avaliable_size.GetX(), ui_scale)
                    + padding_bottom.Evaluate(avaliable_size.GetY(), avaliable_size.GetX(), ui_scale)
                    );
            }
        },
            in_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "desired size text"));

        DscDag::LinkIndexNodes(0, in_avaliable_size, node);
        DscDag::LinkIndexNodes(1, DscDag::DagNodeGroup::GetNodeTokenEnum(in_resource_node_group, DscUi::TUiComponentResourceNodeGroup::TText), node);
        DscDag::LinkIndexNodes(2, DscDag::DagNodeGroup::GetNodeTokenEnum(in_resource_node_group, DscUi::TUiComponentResourceNodeGroup::TUiScale), node);

        DscDag::LinkIndexNodes(3, DscDag::DagNodeGroup::GetNodeTokenEnum(in_resource_node_group, DscUi::TUiComponentResourceNodeGroup::TPaddingLeft), node);
        DscDag::LinkIndexNodes(4, DscDag::DagNodeGroup::GetNodeTokenEnum(in_resource_node_group, DscUi::TUiComponentResourceNodeGroup::TPaddingTop), node);
        DscDag::LinkIndexNodes(5, DscDag::DagNodeGroup::GetNodeTokenEnum(in_resource_node_group, DscUi::TUiComponentResourceNodeGroup::TPaddingRight), node);
        DscDag::LinkIndexNodes(6, DscDag::DagNodeGroup::GetNodeTokenEnum(in_resource_node_group, DscUi::TUiComponentResourceNodeGroup::TPaddingBottom), node);

    }

    return node;
}

DscDag::NodeToken DscUi::MakeNode::MakeRenderRequestSize(
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_desired_size,
    DscDag::NodeToken in_geometry_size_size,
    DscDag::IDagOwner* const in_owner
    )
{
    DscDag::NodeToken node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscCommon::VectorInt2& desired_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            const DscCommon::VectorInt2& geometry_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

            value.Set(
                std::max(desired_size.GetX(), geometry_size.GetX()),
                std::max(desired_size.GetY(), geometry_size.GetY())
            );
        },
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "render request size"));

    DscDag::LinkIndexNodes(0, in_desired_size, node);
    DscDag::LinkIndexNodes(1, in_geometry_size_size, node);
    return node;
}

DscDag::NodeToken DscUi::MakeNode::MakeGeometrySize(
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_component_resource_group,
    DscDag::NodeToken in_parent_component_resource_group,
    DscDag::NodeToken in_desired_size,
    DscDag::NodeToken in_avaliable_size,
    DscDag::IDagOwner* const in_owner
)
{
    DSC_UNUSED(in_dag_collection);
    DSC_UNUSED(in_owner);
    DscDag::NodeToken node = in_desired_size;
    if (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TFlow))
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscUi::TUiFlow flow = DscDag::GetValueType<DscUi::TUiFlow>(in_input_array[0]);
            const DscCommon::VectorInt2& desired_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            const DscCommon::VectorInt2& avaliable_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[2]);

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
            in_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "geometry size flow child"));

        DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TFlow), node);
        DscDag::LinkIndexNodes(1, in_desired_size, node);
        DscDag::LinkIndexNodes(2, in_avaliable_size, node);

    } 
    // do we need to reduce the geoemtry size for the padding?
    else if (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TChildSlotSize))
        //((nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TChildSlotSize)) ||
        //(nullptr != in_component_resource_group.GetNodeToken(DscUi::TUiComponentResourceNodeGroup::TPaddingLeft)))
    {
        // if we have a child slot, presume we are behaving like a canvas that has the geometry size the same as the avaliable size
        node = in_avaliable_size;
    }
    else
    {
        DSC_ASSERT(nullptr != node, "invalid state");
    }

    return node;
}

DscDag::NodeToken DscUi::MakeNode::MakeGeometryOffset(
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_component_resource_group,
    DscDag::NodeToken in_parent_component_resource_group,
    DscDag::NodeToken in_parent_avaliable_size,
    DscDag::NodeToken in_ui_scale,
    DscDag::NodeToken in_geometry_size,
    DscDag::NodeToken in_parent_array_child_ui_node_group,
    DscDag::IDagOwner* const in_owner
)
{
    DSC_UNUSED(in_parent_array_child_ui_node_group);

    DscDag::NodeToken node = nullptr;
    if (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TChildSlotSize))
    {
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscCommon::VectorInt2& parent_avaliable_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            const float ui_scale = DscDag::GetValueType<float>(in_input_array[1]);
            const DscCommon::VectorInt2& geometry_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[2]);

            const DscUi::VectorUiCoord2& child_slot_pivot = DscDag::GetValueType<DscUi::VectorUiCoord2>(in_input_array[3]);
            const DscUi::VectorUiCoord2& child_slot_parent_attach = DscDag::GetValueType<DscUi::VectorUiCoord2>(in_input_array[4]);

            const DscCommon::VectorInt2 pivot_point = child_slot_pivot.EvalueUICoord(geometry_size, ui_scale);
            const DscCommon::VectorInt2 attach_point = child_slot_parent_attach.EvalueUICoord(parent_avaliable_size, ui_scale);

            value.Set(
                attach_point.GetX() - pivot_point.GetX(),
                attach_point.GetY() - pivot_point.GetY()
            );
        },
            in_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "geometry offset child slot"));

        DscDag::LinkIndexNodes(0, in_parent_avaliable_size, node);
        DscDag::LinkIndexNodes(1, in_ui_scale, node);
        DscDag::LinkIndexNodes(2, in_geometry_size, node); // should this be avaliable size or geometry size
        DscDag::LinkIndexNodes(3, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TChildSlotPivot), node);
        DscDag::LinkIndexNodes(4, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TChildSlotParentAttach), node);
    }
    else if (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TFlow))
    {
        // this is a bit bad, we don't have an easy way of knowing what array child index we are of our parent (in a fashion resiliant to array size change) 
        // we could use the geoemtry size node as a token to identify ourself since we have that node on hand...
        //TUiNodeGroup::TGeometrySize
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscDag::NodeToken self_geometry_size = in_input_array[0];
            const std::vector<DscDag::NodeToken>& parent_child_array = DscDag::GetValueNodeArray(in_input_array[1]);
            const DscUi::UiCoord& gap = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[2]);
            const DscCommon::VectorInt2& parent_avaliable_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[3]);
            const float ui_scale = DscDag::GetValueType<float>(in_input_array[4]);
            const DscUi::TUiFlow flow = DscDag::GetValueType<DscUi::TUiFlow>(in_input_array[5]);

            value = {};
            int32 trace = 0;
            for (const auto& item : parent_child_array)
            {
                if (self_geometry_size == DscDag::DagNodeGroup::GetNodeTokenEnum(item, DscUi::TUiNodeGroup::TGeometrySize))
                {
                    break;
                }
                const DscCommon::VectorInt2& geometry_size = DscDag::GetValueType<DscCommon::VectorInt2>(DscDag::DagNodeGroup::GetNodeTokenEnum(item, DscUi::TUiNodeGroup::TGeometrySize));
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

            const DscCommon::VectorInt2& geometry_size = DscDag::GetValueType<DscCommon::VectorInt2>(self_geometry_size);
            switch (flow)
            {
            default:
                break;
            case DscUi::TUiFlow::THorizontal:
            {
                int32 offset = 0;
                if (7 < in_input_array.size())
                {
                    const DscUi::UiCoord& pivot = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[6]);
                    const DscUi::UiCoord& parent_attach = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[7]);
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
                    const DscUi::UiCoord& pivot = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[6]);
                    const DscUi::UiCoord& parent_attach = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[7]);
                    offset = parent_attach.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale) -
                        pivot.Evaluate(geometry_size.GetX(), geometry_size.GetY(), ui_scale);
                }
                value.Set(offset, trace);
            }
            break;
            }

            return;
        },
            in_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "geometry offset child stack"));

        DscDag::LinkIndexNodes(0, in_geometry_size, node);
        DscDag::LinkIndexNodes(1, in_parent_array_child_ui_node_group, node);
        DSC_ASSERT(nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TGap), "parent stack is obliged to have a gap node");
        DscDag::LinkIndexNodes(2, DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TGap), node);
        DscDag::LinkIndexNodes(3, in_parent_avaliable_size, node);
        DscDag::LinkIndexNodes(4, in_ui_scale, node);
        DscDag::LinkIndexNodes(5, DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TFlow), node);
        if ((nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TChildStackPivot)) &&
            (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TChildStackParentAttach)))
        {
            DscDag::LinkIndexNodes(6, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TChildStackPivot), node);
            DscDag::LinkIndexNodes(7, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TChildStackParentAttach), node);
        }
    }
    else if ((nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TPaddingLeft)) &&
        (nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TPaddingTop)))
    {
        // using Parent avaliable size to match what is done in the other usage of padding
        node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscCommon::VectorInt2& parent_avaliable_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
            const float ui_scale = DscDag::GetValueType<float>(in_input_array[1]);
            const DscUi::UiCoord& padding_left = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[2]);
            const DscUi::UiCoord& padding_top = DscDag::GetValueType<DscUi::UiCoord>(in_input_array[3]);

            value.Set(
                padding_left.Evaluate(parent_avaliable_size.GetX(), parent_avaliable_size.GetY(), ui_scale),
                padding_top.Evaluate(parent_avaliable_size.GetY(), parent_avaliable_size.GetX(), ui_scale)
            );
        },
            in_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "geometry offset padding"));

        DscDag::LinkIndexNodes(0, in_parent_avaliable_size, node);
        DscDag::LinkIndexNodes(1, in_ui_scale, node);
        DscDag::LinkIndexNodes(2, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TPaddingLeft), node);
        DscDag::LinkIndexNodes(3, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TPaddingTop), node);
    }
    else
    {
        node = in_dag_collection.CreateValueOnValueChange<DscCommon::VectorInt2>(DscCommon::VectorInt2::s_zero, in_owner);
        DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "geometry offset zero"));
    }

    return node;
}

DscDag::NodeToken DscUi::MakeNode::MakeNodePixelTraversal(
    DscDag::DagCollection& in_dag_collection, 
    DscDag::NodeToken in_geometry_size, 
    DscDag::NodeToken in_render_request_size,
    DscDag::IDagOwner* const in_owner
    )
{
    DscDag::NodeToken node = in_dag_collection.CreateCalculate<DscCommon::VectorInt2>([](DscCommon::VectorInt2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
        const DscCommon::VectorInt2& geometry_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[0]);
        const DscCommon::VectorInt2& render_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

        value.Set(
            std::max(0, render_size.GetX() - geometry_size.GetX()),
            std::max(0, render_size.GetY() - geometry_size.GetY())
        );
    },
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "pixel traversal"));

    DscDag::LinkIndexNodes(0, in_geometry_size, node);
    DscDag::LinkIndexNodes(1, in_render_request_size, node);

    return node;
}

/// note: the automatic scroll, to do pingpoing without state, traverses a range of [-1 ... 1] and is passed through an abs() function
/// otherwise would need state of going up or going down and swap at [0, 1]
/// this works, but manual scroll needs to clamp it's range [0 ... 1] else in negative domain, will be pingpong to positive....
DscDag::NodeToken DscUi::MakeNode::MakeNodeScrollValue(
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_component_resource_group,
    DscDag::NodeToken in_time_delta,
    DscDag::NodeToken in_pixel_traversal_node,
    DscDag::IDagOwner* const in_owner
)
{
    DscDag::NodeToken condition_x = in_dag_collection.CreateCalculate<bool>([](bool& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            // if manual scroll is false and the 0 < pixel traversal, return true
            const bool has_manual_scroll = DscDag::GetValueType<bool>(in_input_array[0]);
            const DscCommon::VectorInt2& pixel_traversal = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

            out_value = ((false == has_manual_scroll) && (0 < pixel_traversal.GetX()));
        },
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(condition_x, "scroll condition x"));
    DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::THasManualScrollX), condition_x);
    DscDag::LinkIndexNodes(1, in_pixel_traversal_node, condition_x);

    DscDag::NodeToken condition_y = in_dag_collection.CreateCalculate<bool>([](bool& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            // if manual scroll is false and the 0 < pixel traversal, return true
            const bool has_manual_scroll = DscDag::GetValueType<bool>(in_input_array[0]);
            const DscCommon::VectorInt2& pixel_traversal = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);

            out_value = ((false == has_manual_scroll) && (0 < pixel_traversal.GetY()));
        },
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(condition_y, "scroll condition y"));
    DscDag::LinkIndexNodes(0, DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::THasManualScrollY), condition_y);
    DscDag::LinkIndexNodes(1, in_pixel_traversal_node, condition_y);

    DscDag::NodeToken tick_scroll_x = in_dag_collection.CreateCalculate<float>([](float& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
        const float time_delta = DscDag::GetValueType<float>(in_input_array[0]);
        const float time_delta_clamped = std::max(0.0f, std::min(1.0f, time_delta));
        const int32 pixel_traversal = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]).GetX();

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
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(tick_scroll_x, "tick scroll x"));
    DscDag::LinkIndexNodes(0, in_time_delta, tick_scroll_x);
    DscDag::LinkIndexNodes(1, in_pixel_traversal_node, tick_scroll_x);

    DscDag::NodeToken tick_scroll_y = in_dag_collection.CreateCalculate<float>([](float& out_value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
        const float time_delta = DscDag::GetValueType<float>(in_input_array[0]);
        const float time_delta_clamped = std::max(0.0f, std::min(1.0f, time_delta));
        const int32 pixel_traversal = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]).GetY();

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
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(tick_scroll_y, "tick scroll y"));
    DscDag::LinkIndexNodes(0, in_time_delta, tick_scroll_y);
    DscDag::LinkIndexNodes(1, in_pixel_traversal_node, tick_scroll_y);

    // something to hold the scroll value output
    DscDag::NodeToken scroll_x = in_dag_collection.CreateValueOnValueChange(0.0f, in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(scroll_x, "scroll_x"));
    DscDag::NodeToken scroll_y = in_dag_collection.CreateValueOnValueChange(0.0f, in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(scroll_y, "scroll_y"));

    DscDag::NodeToken condition_scroll_x = in_dag_collection.CreateCondition<float, float>(
        condition_x,
        tick_scroll_x,
        DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TManualScrollX),
        scroll_x,
        scroll_x,
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(condition_scroll_x, "condition_scroll_x"));
    DSC_UNUSED(condition_scroll_x);

    DscDag::NodeToken condition_scroll_y = in_dag_collection.CreateCondition<float, float>(
        condition_y,
        tick_scroll_y,
        DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TManualScrollY),
        scroll_y,
        scroll_y,
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(condition_scroll_y, "condition_scroll_y"));
    DSC_UNUSED(condition_scroll_y);

    DscDag::NodeToken result_node = in_dag_collection.CreateCalculate<DscCommon::VectorFloat2>([](DscCommon::VectorFloat2& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const float x = DscDag::GetValueType<float>(in_input_array[0]);
            const float y = DscDag::GetValueType<float>(in_input_array[1]);

            //DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "scroll x:%f y:%f\n", x, y);

            value.Set(x, y);
        },
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(result_node, "scroll"));

    DscDag::LinkIndexNodes(0, scroll_x, result_node);
    DscDag::LinkIndexNodes(1, scroll_y, result_node);

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
    DscDag::IDagOwner* const in_owner
)
{
    DscDag::NodeToken node = in_dag_collection.CreateCalculate<DscUi::ScreenSpace>([](DscUi::ScreenSpace& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const DscUi::ScreenSpace& parent_screen_space = DscDag::GetValueType<DscUi::ScreenSpace>(in_input_array[0]);
            const DscCommon::VectorInt2& parent_render_request_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[1]);
            const DscCommon::VectorInt2& geometry_offset = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[2]);
            const DscCommon::VectorInt2& geometry_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[3]);
            const DscCommon::VectorInt2& render_request_size = DscDag::GetValueType<DscCommon::VectorInt2>(in_input_array[4]);
            const DscCommon::VectorFloat2& scroll = DscDag::GetValueType<DscCommon::VectorFloat2>(in_input_array[5]);

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
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "screen space"));

    DscDag::LinkIndexNodes(0, in_parent_screen_space, node);
    DscDag::LinkIndexNodes(1, in_parent_render_request_size, node);
    DscDag::LinkIndexNodes(2, in_geometry_offset, node);
    DscDag::LinkIndexNodes(3, in_geometry_size, node);
    DscDag::LinkIndexNodes(4, in_render_request_size, node);
    DscDag::LinkIndexNodes(5, in_scroll, node);

    return node;
}


DscDag::NodeToken DscUi::MakeNode::MakeLerpFloat4(
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_amount,
    DscDag::NodeToken in_value_0,
    DscDag::NodeToken in_value_1,
    DscDag::IDagOwner* const in_owner
)
{
    DscDag::NodeToken node = in_dag_collection.CreateCalculate<DscCommon::VectorFloat4>([](DscCommon::VectorFloat4& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
        const float amount = DscDag::GetValueType<float>(in_input_array[0]);
        const DscCommon::VectorFloat4& value_0 = DscDag::GetValueType<DscCommon::VectorFloat4>(in_input_array[1]);
        const DscCommon::VectorFloat4& value_1 = DscDag::GetValueType<DscCommon::VectorFloat4>(in_input_array[2]);

        value = value_0 + ((value_1 - value_0) * amount);
    },
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(node, "lerp float4"));

    DscDag::LinkIndexNodes(0, in_amount, node);
    DscDag::LinkIndexNodes(1, in_value_0, node);
    DscDag::LinkIndexNodes(2, in_value_1, node);

    return node;
}

void DscUi::MakeNode::MakeEffectParamTintBlotNode(
    DscDag::NodeToken& out_effect_param,
    DscDag::NodeToken& out_effect_tint,
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_root_node_group,
    DscDag::NodeToken in_parent_node_group,
    const UiManager::TEffectConstructionHelper& in_effect_data,
    DscDag::IDagOwner* const in_owner
)
{
    out_effect_tint = in_dag_collection.CreateValueOnValueChange(
        in_effect_data._effect_param_tint,
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(out_effect_tint, "effect tint burn blot"));

    // if rollover amount changes, we want to hook up the effect param to time delta

    DscDag::NodeToken parent_component_resource_group = DscDag::DagNodeGroup::GetNodeTokenEnum(in_parent_node_group, TUiNodeGroup::TUiComponentResources);
    DscDag::NodeToken condition_node = in_dag_collection.CreateCalculate<bool>([](bool& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const float rollover_amount = DscDag::GetValueType<float>(in_input_array[0]);
            value = (0.0f != rollover_amount);
            // true: link
            // false: unlink
        },
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(condition_node, "effect blot condition"));

    // so, this could be the input flag state, but feels like we want the animation to extend AFTER the rollover finishes, so use the accumulate value
    DscDag::NodeToken rollover_accumulate_node = DscDag::DagNodeGroup::GetNodeTokenEnum(parent_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TInputRolloverAccumulate);
    DSC_ASSERT(nullptr != rollover_accumulate_node, "invalid state");
    DscDag::LinkIndexNodes(0, rollover_accumulate_node, condition_node);

    auto effect_param_time_delta_node = in_dag_collection.CreateValueNotZero(
        0.0f,
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(effect_param_time_delta_node, "effect blot time delta"));

    auto condition = in_dag_collection.CreateCondition<float, bool>(
        condition_node,
        DscDag::DagNodeGroup::GetNodeTokenEnum(in_root_node_group, TUiRootNodeGroup::TTimeDelta),
        nullptr,
        effect_param_time_delta_node,
        nullptr,
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(condition, "effect blot condition node"));
    DSC_UNUSED(condition);

    out_effect_param = in_dag_collection.CreateCalculate<DscCommon::VectorFloat4>([](DscCommon::VectorFloat4& value, std::set<DscDag::NodeToken>&, std::vector<DscDag::NodeToken>& in_input_array) {
            const float time_delta = DscDag::GetValueType<float>(in_input_array[0]);
            const DscUi::TUiInputStateFlag input_state_flag = DscDag::GetValueType<DscUi::TUiInputStateFlag>(in_input_array[1]);
            const float rollover_flag = (0 != (input_state_flag & DscUi::TUiInputStateFlag::TRollover)) ? 1.0f : 0.0f;
            const DscCommon::VectorFloat2& touch_pos = DscDag::GetValueType<DscCommon::VectorFloat2>(in_input_array[2]);

            // x. rolled over [0 ... 1] (1 == rolled over), y. time delta [0 ...], zw. mouse pos relative to shader in pixels, uv coords bottom left is 0,0
            value.Set(
                rollover_flag,
                time_delta,
                touch_pos.GetX(),
                touch_pos.GetY()
            );
            //DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_UI, "%f %f %f %f\n", value[0], value[1], value[2], value[3]);
        },
        in_owner);
    DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(out_effect_param, "effect param burn blot"));

    DscDag::LinkIndexNodes(0, effect_param_time_delta_node, out_effect_param);
    DscDag::NodeToken input_state_node = DscDag::DagNodeGroup::GetNodeTokenEnum(parent_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TInputStateFlag);
    DSC_ASSERT(nullptr != input_state_node, "invalid state");
    DscDag::LinkIndexNodes(1, input_state_node, out_effect_param);
    DscDag::NodeToken active_touch_pos_node = DscDag::DagNodeGroup::GetNodeTokenEnum(parent_component_resource_group, DscUi::TUiComponentResourceNodeGroup::TInputActiveTouchPos);
    DSC_ASSERT(nullptr != active_touch_pos_node, "invalid state");
    DscDag::LinkIndexNodes(2, active_touch_pos_node, out_effect_param);

    return;
}

void DscUi::MakeNode::MakeEffectParamTintNode(
    DscDag::NodeToken& out_effect_strength,
    DscDag::NodeToken& out_effect_param,
    DscDag::NodeToken& out_effect_tint,
    DscDag::DagCollection& in_dag_collection,
    DscDag::NodeToken in_component_resource_group,
    const UiManager::TEffectConstructionHelper& in_effect_data,
    DscDag::IDagOwner* const in_owner
    )
{
    if (true == in_effect_data._use_rollover_param_lerp)
    {
        DSC_ASSERT(nullptr != DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TInputRolloverAccumulate), "invalid state");

        DscDag::NodeToken effect_param_0 = in_dag_collection.CreateValueOnValueChange(
            in_effect_data._effect_param,
            in_owner);
            DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(effect_param_0, "effect param 0"));
        DscDag::NodeToken effect_tint_0 = in_dag_collection.CreateValue(
            in_effect_data._effect_param_tint,
            DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
            in_owner);
            DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(effect_tint_0, "effect tint 0"));
        DscDag::NodeToken effect_param_1 = in_dag_collection.CreateValue(
            in_effect_data._effect_param_rollover,
            DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
            in_owner);
            DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(effect_param_1, "effect param 1"));
        DscDag::NodeToken effect_tint_1 = in_dag_collection.CreateValue(
            in_effect_data._effect_param_tint_rollover,
            DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
            in_owner);
            DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(effect_tint_1, "effect tint 1"));

        out_effect_param = MakeNode::MakeLerpFloat4(
            in_dag_collection,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TInputRolloverAccumulate),
            effect_param_0,
            effect_param_1,
            in_owner
        );

        out_effect_tint = MakeNode::MakeLerpFloat4(
            in_dag_collection,
            DscDag::DagNodeGroup::GetNodeTokenEnum(in_component_resource_group, TUiComponentResourceNodeGroup::TInputRolloverAccumulate),
            effect_tint_0,
            effect_tint_1,
            in_owner
        );
    }
    else
    {
        out_effect_param = in_dag_collection.CreateValue(
            in_effect_data._effect_param,
            DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
            in_owner);
            DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(out_effect_param, "effect param"));
        out_effect_tint = in_dag_collection.CreateValue(
            in_effect_data._effect_param_tint,
            DscDag::CallbackOnValueChange<DscCommon::VectorFloat4>::Function,
            in_owner);
            DSC_DEBUG_ONLY(DscDag::DebugSetNodeName(out_effect_tint, "effect tint"));
    }

    out_effect_strength = DscDag::DagNodeGroup::GetNodeTokenEnum(
        in_component_resource_group,
        DscUi::TUiComponentResourceNodeGroup::TEffectStrength
    );

    return;
}


