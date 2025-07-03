#include "ui_input_state.h"

DscUi::UiInputState::UiInputState()
{
	//nop
}

// mouse pos in pixels, relative to 
void DscUi::UiInputState::SetMouseTouch(
	const DscCommon::VectorInt2&,
	const bool,
	const bool
)
{
	//nop
}

// return true if values changed
const bool DscUi::UiInputState::Update(const UiInputState&)
{
	return true;
}

const bool DscUi::UiInputState::operator==(const UiInputState&) const
{
	return false;
}
const bool DscUi::UiInputState::operator!=(const UiInputState& in_rhs) const
{
	return !operator==(in_rhs);
}

