#pragma once

#define LOG_TOPIC_DSC_UI "DSC_UI"

#ifndef DSC_BFF_BUILD

#include "..\..\dsc_common\dsc_common\dsc_common.h"
#include "..\..\dsc_dag\dsc_dag\dsc_dag.h"
#include "..\..\dsc_dag_render\dsc_dag_render\dsc_dag_render.h"
#include "..\..\dsc_locale\dsc_locale\dsc_locale.h"
#include "..\..\dsc_render\dsc_render\dsc_render.h"
#include "..\..\dsc_render_resource\dsc_render_resource\dsc_render_resource.h"
#include "screen_quad.h"
#include "ui_component_canvas.h"
#include "ui_component_debug_fill.h"
#include "ui_coord.h"
#include "ui_dag_node_component.h"
#include "ui_enum.h"
#include "ui_manager.h"
#include "vector_ui_coord2.h"

#endif //#ifndef DSC_BFF_BUILD

#include <dsc_common/dsc_common.h>

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
	struct TSizeShaderConstantBuffer
	{
		float _value[4]; // _width_height
	};

	struct TUiPanelShaderConstantBuffer
	{
		float _pos_size[4]; // _pos_x_y_size_width_height;
		float _uv_size[4]; // _ui_x_y_size_width_height;
		//float _tint_colour[4];

		//void Calculate(
		//	const DscCommon::VectorInt2& in_parent_render_size,
		//	const DscCommon::VectorInt2& in_geometry_offset,
		//	const DscCommon::VectorInt2& in_geometry_size,
		//	const DscCommon::VectorInt2& in_render_size,
		//	const DscCommon::VectorFloat2& in_scroll_value
		//);
	};
}

