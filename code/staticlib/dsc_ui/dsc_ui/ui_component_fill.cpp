#include "ui_component_fill.h"
#include "ui_manager.h"
#include <dsc_render_resource\frame.h>
#include <dsc_common\vector_int2.h>
#include <dsc_common\vector_float4.h>
#include <dsc_dag\dag_collection.h>

DscUi::UiComponentFill::UiComponentFill()
{
	// nop
}

void DscUi::UiComponentFill::Draw(
	DscRenderResource::Frame& in_frame,
	DscRender::IRenderTarget& in_render_target,
	const float
)
{
	in_frame.SetRenderTarget(&in_render_target);
	in_frame.SetRenderTarget(nullptr);
}
