#include "ui_component_effect_round_corner.h"
#include "ui_manager.h"
#include <dsc_render\draw_system.h>
#include <dsc_render\i_render_target.h>
#include <dsc_render_resource\frame.h>
#include <dsc_text\text_manager.h>
#include <dsc_text\text_run.h>
#include <dsc_dag\dag_collection.h>
#include <dsc_dag\dag_group.h>

DscUi::UiComponentEffectRoundCorner::UiComponentEffectRoundCorner(
	const std::shared_ptr<DscRenderResource::Shader>& in_shader,
	const std::shared_ptr<DscRenderResource::ShaderConstantBuffer>& in_shader_constant_buffer,
	const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_full_target_quad,
	const float in_pixel_radius
)
	: _shader(in_shader)
	, _shader_constant_buffer(in_shader_constant_buffer)
	, _full_target_quad(in_full_target_quad)
	, _pixel_radius(in_pixel_radius)
{
	// nop
}
void DscUi::UiComponentEffectRoundCorner::Draw(
	DscRenderResource::Frame& in_frame,
	DscRender::IRenderTarget& in_render_target,
	const float
)
{
	DSC_ASSERT(nullptr != _shader, "invalid state");
	DSC_ASSERT(nullptr != _shader_constant_buffer, "invalid state");
	DSC_ASSERT(nullptr != _full_target_quad, "invalid state");

	const DscCommon::VectorInt2 viewport_size = in_render_target.GetViewportSize();
	{
		DscUi::TEffectConstantBuffer& buffer = _shader_constant_buffer->GetConstant<DscUi::TEffectConstantBuffer>(0);
		buffer._value[0] = static_cast<float>(viewport_size.GetX());
		buffer._value[1] = static_cast<float>(viewport_size.GetY());
		buffer._param[0] = _pixel_radius;
		buffer._param[1] = _pixel_radius;
		buffer._param[2] = _pixel_radius;
		buffer._param[3] = _pixel_radius;
	}

	in_frame.SetRenderTarget(&in_render_target);
	in_frame.SetShader(_shader, _shader_constant_buffer);
	in_frame.Draw(_full_target_quad);
	in_frame.SetRenderTarget(nullptr);
}

void DscUi::UiComponentEffectRoundCorner::SetClearColour(const DscCommon::VectorFloat4& in_colour)
{
	DscDag::DagCollection::SetValueType(_ui_component_group.GetNodeToken(TUiComponentGroup::TClearColourNode), in_colour);
	return;
}

void DscUi::UiComponentEffectRoundCorner::SetParentChildIndex(const int32 in_parent_child_index)
{
	DscDag::DagCollection::SetValueType<int32>(_ui_component_group.GetNodeToken(TUiComponentGroup::TParentChildIndex), in_parent_child_index);
	return;
}

void DscUi::UiComponentEffectRoundCorner::SetNode(const DagGroupUiComponent& in_ui_component_group)
{
	_ui_component_group = in_ui_component_group;
}