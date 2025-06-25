#include "ui_component_fill.h"
#include "ui_manager.h"
#include <dsc_render_resource\frame.h>
#include <dsc_common\vector_int2.h>
#include <dsc_dag\dag_collection.h>

DscUi::UiComponentFill::UiComponentFill()
{
	// nop
}

void DscUi::UiComponentFill::Draw(
	DscRenderResource::Frame& in_frame,
	DscRender::IRenderTarget& in_render_target,
	const float
)
{
	in_frame.SetRenderTarget(&in_render_target);
	in_frame.SetRenderTarget(nullptr);
}

void DscUi::UiComponentFill::SetClearColour(const DscCommon::VectorFloat4& in_colour)
{
	DSC_ASSERT(nullptr != _clear_colour_node, "invalid state");
	DscDag::DagCollection::SetValueType(_clear_colour_node, in_colour);
	return;
}

void DscUi::UiComponentFill::SetParentChildIndex(const int32 in_parent_child_index)
{
	DSC_ASSERT(nullptr != _parent_child_index, "invalid state");
	DscDag::DagCollection::SetValueType<int32>(_parent_child_index, in_parent_child_index);
	return;
}

void DscUi::UiComponentFill::SetNode(
	DscDag::NodeToken in_parent_child_index,
	DscDag::NodeToken in_clear_colour_node,
	DscDag::NodeToken,
	DscDag::NodeToken
)
{
	_parent_child_index = in_parent_child_index;
	_clear_colour_node = in_clear_colour_node;
	return;
}
