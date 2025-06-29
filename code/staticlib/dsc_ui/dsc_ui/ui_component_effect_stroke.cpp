#include "ui_component_effect_stroke.h"
#include "ui_manager.h"
#include <dsc_render\draw_system.h>
#include <dsc_text\text_manager.h>
#include <dsc_text\text_run.h>
#include <dsc_dag\dag_collection.h>
#include <dsc_dag\dag_group.h>
#include <dsc_render\i_render_target.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\geometry_generic.h>
#include <dsc_render_resource\shader.h>
#include <dsc_render_resource\shader_constant_buffer.h>
#include <dsc_render_resource\render_target_texture.h>

DscUi::UiComponentEffectStroke::UiComponentEffectStroke(
	const std::shared_ptr<DscRenderResource::Shader>& in_shader,
	const std::shared_ptr<DscRenderResource::ShaderConstantBuffer>& in_shader_constant_buffer,
	const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_full_target_quad,
	const DscCommon::VectorFloat4& in_param,
	const DscCommon::VectorFloat4& in_stroke_colour
)
	: _shader(in_shader)
	, _shader_constant_buffer(in_shader_constant_buffer)
	, _full_target_quad(in_full_target_quad)
	, _param(in_param)
	, _stroke_colour(in_stroke_colour)
{
	// nop
}

void DscUi::UiComponentEffectStroke::Draw(
	DscRenderResource::Frame& in_frame,
	DscRender::IRenderTarget& in_render_target,
	const float in_ui_scale
)
{
	DSC_ASSERT(nullptr != _shader, "invalid state");
	DSC_ASSERT(nullptr != _shader_constant_buffer, "invalid state");
	DSC_ASSERT(nullptr != _full_target_quad, "invalid state");

	std::shared_ptr<DscRenderResource::RenderTargetTexture> render_texture = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::RenderTargetTexture>>(_child_render_node);

	_shader->SetShaderResourceViewHandle(0, render_texture->GetShaderResourceHeapWrapperItem(0));

	const DscCommon::VectorInt2 viewport_size = in_render_target.GetViewportSize();
	{
		DscUi::TEffectConstantBuffer& buffer = _shader_constant_buffer->GetConstant<DscUi::TEffectConstantBuffer>(0);
		buffer._width_height[0] = static_cast<float>(viewport_size.GetX());
		buffer._width_height[1] = static_cast<float>(viewport_size.GetY());
		buffer._effect_param[0] = _param[0];
		buffer._effect_param[1] = _param[1];
		buffer._effect_param[2] = _param[2] * in_ui_scale;
		buffer._effect_param[3] = _param[3];
		buffer._tint_colour[0] = _stroke_colour[0];
		buffer._tint_colour[1] = _stroke_colour[1];
		buffer._tint_colour[2] = _stroke_colour[2];
		buffer._tint_colour[3] = _stroke_colour[3];
		buffer._texture_param_0[0] = static_cast<float>(render_texture->GetViewportSize().GetX());
		buffer._texture_param_0[1] = static_cast<float>(render_texture->GetViewportSize().GetY());
		buffer._texture_param_0[2] = static_cast<float>(render_texture->GetSize().GetX());
		buffer._texture_param_0[3] = static_cast<float>(render_texture->GetSize().GetY());
	}

	in_frame.SetRenderTarget(&in_render_target);
	in_frame.SetShader(_shader, _shader_constant_buffer);
	in_frame.Draw(_full_target_quad);
	in_frame.SetRenderTarget(nullptr);
}

void DscUi::UiComponentEffectStroke::AddChild(
	IUiComponent* const in_child_component, // we don't keep a reference, we just set the parent child index to zero
	DscDag::NodeToken in_render_node
)
{
	in_child_component->SetParentChildIndex(0);
	_child_render_node = in_render_node;
	return;
}

void DscUi::UiComponentEffectStroke::SetClearColour(const DscCommon::VectorFloat4& in_colour)
{
	DscDag::DagCollection::SetValueType(_ui_component_group.GetNodeToken(TUiComponentGroup::TClearColourNode), in_colour);
	return;
}

void DscUi::UiComponentEffectStroke::SetParentChildIndex(const int32 in_parent_child_index)
{
	DscDag::DagCollection::SetValueType<int32>(_ui_component_group.GetNodeToken(TUiComponentGroup::TParentChildIndex), in_parent_child_index);
	return;
}

void DscUi::UiComponentEffectStroke::SetNode(const DagGroupUiComponent& in_ui_component_group)
{
	_ui_component_group = in_ui_component_group;
}