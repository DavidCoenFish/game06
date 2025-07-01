#include "ui_component_button.h"
#include "ui_manager.h"
#include <dsc_render\draw_system.h>
#include <dsc_render\i_render_target.h>
#include <dsc_render_resource\frame.h>
#include <dsc_text\text_manager.h>
#include <dsc_text\text_run.h>
#include <dsc_dag\dag_collection.h>
#include <dsc_dag\dag_group.h>

DscUi::UiComponentButton::UiComponentButton(
	const TUiComponentBehaviour in_behaviour
	)
	: _behaviour(in_behaviour)
{
	// nop
}

void DscUi::UiComponentButton::Draw(
	DscRenderResource::Frame& in_frame,
	DscRender::IRenderTarget& in_render_target,
	const float
)
{
	in_frame.SetRenderTarget(&in_render_target);

	// select the first appropriate state to draw


	in_frame.SetRenderTarget(nullptr);
}
