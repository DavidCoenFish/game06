#include "ui_input_param.h"

DscUi::UiInputParam::UiInputParam()
{
	//nop
}

// mouse pos in pixels, relative to 
void DscUi::UiInputParam::SetMouseTouch(
	const DscCommon::VectorInt2& in_pos,
	const bool in_left_button_down,
	const bool in_right_button_down
)
{
	DSC_UNUSED(in_right_button_down);
	_touch_data_array.push_back({
		in_pos,
		in_left_button_down,
		0,
		TUiTouchFlavour::TMouseLeft
		});
}
