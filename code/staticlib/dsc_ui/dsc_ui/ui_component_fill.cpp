#include "ui_component_fill.h"
#include "ui_manager.h"


DscUi::UiComponentFill::UiComponentFill(const DscCommon::VectorFloat4& in_fill_colour)
	: _fill_colour(in_fill_colour)
{
	// nop
}

const DscCommon::VectorFloat4& DscUi::UiComponentFill::GetClearColour() const
{
	return _fill_colour;
}
