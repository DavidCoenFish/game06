#include "ui_component_fill.h"
#include "ui_manager.h"


DscUi::UiComponentFill::UiComponentFill(
	const int32 in_parent_child_index,
	const DscCommon::VectorFloat4& in_fill_colour
	)
	: _parent_child_index(in_parent_child_index)
	, _fill_colour(in_fill_colour)
{
	// nop
}

const DscCommon::VectorFloat4& DscUi::UiComponentFill::GetClearColour() const
{
	return _fill_colour;
}

const int32 DscUi::UiComponentFill::GetParentChildIndex() const
{
	return _parent_child_index;
}
