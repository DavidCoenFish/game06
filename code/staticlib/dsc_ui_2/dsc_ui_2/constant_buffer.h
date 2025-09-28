#pragma once
#include "dsc_ui_2.h"

namespace DscUi2
{
	struct TDebugGridConstantBuffer
	{
		float _width_height[4]; // _width_height_screen_left_offset_top_offset (ui coords are top left relative)
	};

	struct TUiPanelShaderConstantBufferVS
	{
		float _pos_size[4]; // _pos_x_y_size_width_height;
		float _uv_size[4]; // _ui_x_y_size_width_height;
	};
	struct TUiPanelShaderConstantBufferPS
	{
		float _tint_colour[4];
	};

} // DscUi