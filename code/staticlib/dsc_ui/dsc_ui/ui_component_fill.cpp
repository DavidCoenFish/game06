#include "ui_component_fill.h"
#include "ui_manager.h"
#include <dsc_render_resource\frame.h>
#include <dsc_common\vector_int2.h>
#include <dsc_common\vector_float4.h>
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
	DscDag::DagCollection::SetValueType(_ui_component_group.GetNodeToken(TUiComponentGroup::TClearColourNode), in_colour);
	return;
}

void DscUi::UiComponentFill::SetParentChildIndex(const int32 in_parent_child_index)
{
	DscDag::DagCollection::SetValueType<int32>(_ui_component_group.GetNodeToken(TUiComponentGroup::TParentChildIndex), in_parent_child_index);
	return;
}

void DscUi::UiComponentFill::SetNode(const DagGroupUiComponent& in_ui_component_group)
{
	_ui_component_group = in_ui_component_group;
	return;
}
