#include "ui_input_state.h"

DscUi::UiInputState::UiInputState()
{
	//nop
}

// if a node is deleted, clear our references to nodes, ref to raw pointer...
void DscUi::UiInputState::ClearNodeReferences()
{
	for (auto& item : _touch_state)
	{
		item._node_under_click_start = nullptr;
	}
}

DscUi::UiInputState::TouchState& DscUi::UiInputState::GetTouchState(const UiInputParam::TouchData& in_touch_data)
{
	TouchState & touch_state = GetTouchStateForId(in_touch_data._id);

	// we want to know the frame that touch action starts and ends, ie, the click state
	if (in_touch_data._active != touch_state._lask_known_touch_active)
	{
		touch_state._lask_known_touch_active = in_touch_data._active;
		touch_state._click_start = in_touch_data._active;
		touch_state._click_end = !in_touch_data._active;
	}
	else
	{
		touch_state._click_start = false;
		touch_state._click_end = false;
	}
	return touch_state;
}

DscUi::UiInputState::TouchState& DscUi::UiInputState::GetTouchStateForId(const int32 in_id)
{
	for (auto& item : _touch_state)
	{
		if (in_id == item._id)
		{
			return item;
		}
	}

	_touch_state.push_back({in_id});
	return _touch_state.back();
}
