#include "ui_component_text.h"
#include "ui_manager.h"
#include <dsc_render\draw_system.h>
#include <dsc_render\i_render_target.h>
#include <dsc_render_resource\frame.h>
#include <dsc_text\text_manager.h>
#include <dsc_text\text_run.h>
#include <dsc_dag\dag_collection.h>
#include <dsc_dag\dag_group.h>

DscUi::UiComponentText::UiComponentText(
	DscText::TextManager& in_text_manager,
	std::unique_ptr<DscText::TextRun>&& in_text_run,
	const TUiComponentBehaviour in_behaviour
	)
	: _text_manager(in_text_manager)
	, _behaviour(in_behaviour)
	, _text_run(std::move(in_text_run))
{
	// nop
}

const DscCommon::VectorInt2 DscUi::UiComponentText::ConvertAvaliableSizeToDesiredSize(const DscCommon::VectorInt2&, const DscCommon::VectorInt2& in_avaliable_size, const float in_ui_scale)
{
	DscCommon::VectorInt2 result = {};
	DscText::TextRun* const text_run_raw = _text_run.get();
	if (nullptr != text_run_raw)
	{
		text_run_raw->SetWidthLimit(
			text_run_raw->GetWidthLimitEnabled(),
			in_avaliable_size.GetX()
			);
		text_run_raw->SetUIScale(in_ui_scale);

		result = text_run_raw->GetTextBounds();
	}
	// TODO: deal with protection against zero size desired size for ui components, possibly you just don't draw the ui component?
	return result;
}

void DscUi::UiComponentText::Draw(
	DscRenderResource::Frame& in_frame,
	DscRender::IRenderTarget& in_render_target,
	const float
)
{
	in_frame.SetRenderTarget(&in_render_target);

	DscText::TextRun* const text_run_raw = _text_run.get();
	if (nullptr != text_run_raw)
	{
		text_run_raw->SetTextContainerSize(in_render_target.GetViewportSize());

		auto geometry = text_run_raw->GetGeometry(&in_frame.GetDrawSystem(), &in_frame);
		auto shader = _text_manager.GetShader(&in_frame.GetDrawSystem(), &in_frame);
		in_frame.SetShader(shader);
		in_frame.Draw(geometry);
	}

	in_frame.SetRenderTarget(nullptr);
}

void DscUi::UiComponentText::SetClearColour(const DscCommon::VectorFloat4& in_colour)
{
	DscDag::DagCollection::SetValueType(_ui_component_group.GetNodeToken(TUiComponentGroup::TClearColourNode), in_colour);
	return;
}

void DscUi::UiComponentText::SetParentChildIndex(const int32 in_parent_child_index)
{
	DscDag::DagCollection::SetValueType<int32>(_ui_component_group.GetNodeToken(TUiComponentGroup::TParentChildIndex), in_parent_child_index);
	return;
}

void DscUi::UiComponentText::SetNode(const DagGroupUiComponent& in_ui_component_group)
{
	_ui_component_group = in_ui_component_group;
	return;
}
