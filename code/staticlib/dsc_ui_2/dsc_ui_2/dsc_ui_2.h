#pragma once

#define LOG_TOPIC_DSC_UI_2 "DSC_UI_2"

#ifndef DSC_BFF_BUILD

#include "..\..\dsc_common\dsc_common\dsc_common.h"
#include "..\..\dsc_dag_2\dsc_dag_2\dsc_dag_2.h"
#include "..\..\dsc_locale\dsc_locale\dsc_locale.h"
#include "..\..\dsc_render\dsc_render\dsc_render.h"
#include "..\..\dsc_render_resource\dsc_render_resource\dsc_render_resource.h"
#include "constant_buffer.h"
#include "coord.h"
#include "enum.h"
#include "manager.h"
#include "vector_coord2.h"

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
