#include "ui_component_fill.h"
#include "ui_manager.h"
#include <dsc_render_resource\frame.h>
#include <dsc_common\vector_int2.h>
#include <dsc_dag\dag_collection.h>

DscUi::UiComponentFill::UiComponentFill(
	const int32 in_parent_child_index,
	const DscCommon::VectorFloat4& in_fill_colour
	)
	: _parent_child_index(in_parent_child_index)
	, _fill_colour(in_fill_colour)
{
	// nop
}

void DscUi::UiComponentFill::Draw(
	DscRenderResource::Frame& in_frame,
	DscRender::IRenderTarget& in_render_target
)
{
	in_frame.SetRenderTarget(&in_render_target);
	in_frame.SetRenderTarget(nullptr);
}

const DscCommon::VectorFloat4& DscUi::UiComponentFill::GetClearColour() const
{
	return _fill_colour;
}

const bool DscUi::UiComponentFill::SetClearColour(const DscCommon::VectorFloat4& in_colour)
{
	bool result = false;
	if (in_colour != _fill_colour)
	{
		result = true;
		_fill_colour = in_colour;
	}
	return result;
}

const int32 DscUi::UiComponentFill::GetParentChildIndex() const
{
	return _parent_child_index;
}

void DscUi::UiComponentFill::Update(const float in_time_delta)
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

void DscUi::UiComponentFill::SetNode(DscDag::NodeToken in_render_node, DscDag::NodeToken in_desired_size_node, DscDag::NodeToken in_pixel_traversal_size_node, DscDag::NodeToken in_scroll_node, DscDag::NodeToken in_ui_panel_shader_constant_node)
{
	_render_node = in_render_node;
	_desired_size_node = in_desired_size_node;
	_pixel_traversal_size_node = in_pixel_traversal_size_node;
	_scroll_node = in_scroll_node;
	_ui_panel_shader_constant_node = in_ui_panel_shader_constant_node;
}

void DscUi::UiComponentFill::BuildUiPanelShaderConstant(TUiPanelShaderConstantBuffer& out_shader_constant_buffer)
{
	out_shader_constant_buffer = DscDag::DagCollection::GetValueType<TUiPanelShaderConstantBuffer>(_ui_panel_shader_constant_node);
}

std::shared_ptr<DscRender::HeapWrapperItem> DscUi::UiComponentFill::GetRenderTexture()
{
	return DscDag::DagCollection::GetValueType<std::shared_ptr<DscRender::HeapWrapperItem>>(_render_node);
}

