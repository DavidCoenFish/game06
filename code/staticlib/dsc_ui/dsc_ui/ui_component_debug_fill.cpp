#include "ui_component_debug_fill.h"
#include "ui_manager.h"
#include <dsc_common\vector_int2.h>
#include <dsc_dag\dag_collection.h>
#include <dsc_render\i_render_target.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\geometry_generic.h>
#include <dsc_render_resource\shader.h>
#include <dsc_render_resource\shader_constant_buffer.h>

DscUi::UiComponentDebugFill::UiComponentDebugFill(
	const int32 in_parent_child_index,
	const std::shared_ptr<DscRenderResource::Shader>& in_shader,
	const std::shared_ptr<DscRenderResource::ShaderConstantBuffer>& in_shader_constant_buffer,
	const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_full_target_quad
)
	: _parent_child_index(in_parent_child_index)
	, _shader(in_shader)
	, _shader_constant_buffer(in_shader_constant_buffer)
	, _full_target_quad(in_full_target_quad)
{
	// nop
}

void DscUi::UiComponentDebugFill::Draw(
	DscRenderResource::Frame& in_frame,
	DscRender::IRenderTarget& in_render_target
)
{
	DSC_ASSERT(nullptr != _shader, "invalid state");
	DSC_ASSERT(nullptr != _shader_constant_buffer, "invalid state");
	DSC_ASSERT(nullptr != _full_target_quad, "invalid state");

	const DscCommon::VectorInt2 viewport_size = in_render_target.GetViewportSize();
	{
		DscUi::TSizeShaderConstantBuffer& buffer = _shader_constant_buffer->GetConstant<DscUi::TSizeShaderConstantBuffer>(0);
		buffer._value[0] = static_cast<float>(viewport_size.GetX());
		buffer._value[1] = static_cast<float>(viewport_size.GetY());
	}

	in_frame.SetRenderTarget(&in_render_target);
	in_frame.SetShader(_shader, _shader_constant_buffer);
	in_frame.Draw(_full_target_quad);
	in_frame.SetRenderTarget(nullptr);
}

const int32 DscUi::UiComponentDebugFill::GetParentChildIndex() const
{
	return _parent_child_index;
}

void DscUi::UiComponentDebugFill::Update(const float in_time_delta)
{
	const float delta = std::max(0.0f, std::min(0.1f, in_time_delta));
	DscCommon::VectorInt2 pixel_range = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(_pixel_traversal_size_node);
	DscCommon::VectorFloat2 scroll = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat2>(_scroll_node);
	float value_x = 0.0f;
	if (0 < pixel_range.GetX())
	{
		value_x = scroll.GetX();
		value_x += (64.0f * delta / static_cast<float>(pixel_range.GetX()));
	}
	float value_y = 0.0f;
	if (0 < pixel_range.GetY())
	{
		value_y = scroll.GetY();
		value_y += (64.0f * delta / static_cast<float>(pixel_range.GetY()));
	}

	DscDag::DagCollection::SetValueType<DscCommon::VectorFloat2>(_scroll_node, DscCommon::VectorFloat2(value_x, value_y));
}

void DscUi::UiComponentDebugFill::SetNode(DscDag::NodeToken in_render_node, DscDag::NodeToken in_desired_size_node, DscDag::NodeToken in_pixel_traversal_size_node, DscDag::NodeToken in_scroll_node, DscDag::NodeToken in_ui_panel_shader_constant_node)
{
	_render_node = in_render_node;
	_desired_size_node = in_desired_size_node;
	_pixel_traversal_size_node = in_pixel_traversal_size_node;
	_scroll_node = in_scroll_node;
	_ui_panel_shader_constant_node = in_ui_panel_shader_constant_node;
}

void DscUi::UiComponentDebugFill::BuildUiPanelShaderConstant(TUiPanelShaderConstantBuffer& out_shader_constant_buffer)
{
	out_shader_constant_buffer = DscDag::DagCollection::GetValueType<TUiPanelShaderConstantBuffer>(_ui_panel_shader_constant_node);
}

std::shared_ptr<DscRender::HeapWrapperItem> DscUi::UiComponentDebugFill::GetRenderTexture()
{
	return DscDag::DagCollection::GetValueType<std::shared_ptr<DscRender::HeapWrapperItem>>(_render_node);
}


