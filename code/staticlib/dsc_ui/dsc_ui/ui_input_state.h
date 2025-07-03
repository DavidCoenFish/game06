#pragma once
#include "dsc_ui.h"

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
}

namespace DscUi
{
	class UiInputState
	{
	public:
		UiInputState();

		// mouse pos in pixels, relative to 
		void SetMouseTouch(
			const DscCommon::VectorInt2& in_pos,
			const bool in_left_button_down,
			const bool in_right_button_down
			);

		// return true if values changed
		const bool Update(const UiInputState& in_ui_input_state);

		const bool operator==(const UiInputState& in_rhs) const;
		const bool operator!=(const UiInputState& in_rhs) const;
	private:
		//array of touch data
	


	};
}



