#pragma once

#define LOG_TOPIC_DSC_UI "DSC_UI"

#ifndef DSC_BFF_BUILD

#include "..\..\dsc_common\dsc_common\dsc_common.h"
#include "..\..\dsc_dag\dsc_dag\dsc_dag.h"
#include "..\..\dsc_dag_render\dsc_dag_render\dsc_dag_render.h"
#include "..\..\dsc_locale\dsc_locale\dsc_locale.h"
#include "..\..\dsc_render\dsc_render\dsc_render.h"
#include "..\..\dsc_render_resource\dsc_render_resource\dsc_render_resource.h"
#include "component_construction_helper.h"
#include "screen_quad.h"
#include "ui_coord.h"
#include "ui_enum.h"
#include "ui_input_param.h"
#include "ui_input_state.h"
#include "ui_manager.h"
#include "ui_render_target.h"
#include "vector_ui_coord2.h"

#endif //#ifndef DSC_BFF_BUILD

#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_float4.h>

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
	typedef Vector2<float> VectorFloat2;
	template <typename TYPE>
	class Vector4;
	typedef Vector4<float> VectorFloat4;
}

namespace DscUi
{
	struct TDebugGridConstantBuffer
	{
		float _width_height[4]; // _width_height_screen_left_offset_top_offset (ui coords are top left relative)
	};

	struct TFillConstantBuffer
	{
		float _colour[4];
	};

	struct TGradientFillConstantBuffer
	{
		// if there is transparency, it will blend with the clear clour of the render target
		// focus is [0...1]  is the expected locations of the colour stops, example [0.0, 0.33, 0.66, 1.0], 
		// expect focus values to be monically increacing and NO COINCIDENT (not equal to each other) can be outside [0...1]
		float _focus[4];
		float _colour_0[4];
		float _colour_1[4];
		float _colour_2[4];
		float _colour_3[4];
		// direction scale not implement, so currently hardcoded to be equivalent to [0, 1, 0, 1]
		//float direction_scale[4]; // vx, vy, d1, d2 -> focus_needle = (dot(uv, [vx, vy]) - d1) / (d2 - d1)
	};
} // DscUi

const bool operator==(const DscUi::TGradientFillConstantBuffer& in_lhs, const DscUi::TGradientFillConstantBuffer& in_rhs);

namespace DscUi
{
	struct TEffectConstantBuffer
	{
		float _width_height[4]; // _width height, screen left offset, // todo: top offset (ui coords are top left relative)
		float _effect_param[4]; // 4 floats for the effect * ui scale
		float _effect_tint[4];
		float _texture_param_0[4]; // viewport width height, texture width height (ui viewport can be smaller than the texture size)
		float _texture_param_1[4]; // viewport width height, texture width height (ui viewport can be smaller than the texture size)
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

}

