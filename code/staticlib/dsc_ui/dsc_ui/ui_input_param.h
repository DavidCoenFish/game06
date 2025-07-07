#pragma once
#include "dsc_ui.h"
#include "ui_enum.h"

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
}

namespace DscUi
{
	class UiInputParam
	{
	public:
		UiInputParam();

		// mouse pos in pixels, relative to 
		void SetMouseTouch(
			const DscCommon::VectorInt2& in_pos,
			const bool in_left_button_down,
			const bool in_right_button_down
			);

		//array of touch data
		struct TouchData
		{
			/// Pixel coords relative to application window top left
			DscCommon::VectorInt2 _root_relative_pos = {};
			/// Mouse button down/ finger touching screen
			bool _active = false;
			/// Each continous touch gets an id, may be zero for mouse move and mouse left button
			int32 _id = 0;
			/// Hint on touch type
			TUiTouchFlavour _flavour = TUiTouchFlavour::TNone;
		};

		std::vector<TouchData> _touch_data_array = {};

		struct NavigationData
		{
			/// Key type
			TUiNavigationType _type;
			/// Duration of key down
			//float _duration; //duration gets calculated internally?
			/// joystick or scroll amount, else 1.0 for keypress?
			float _amount = 0.0f;
			/// True if the key down ended this frame
			bool _end = false;
		};

		std::vector<NavigationData> _navigation_data_array = {};

	};
}



