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
	const TUiComponentBehaviour in_behaviour,
	const bool in_enable_scale,
	const int32 in_scale_threashold,
	const float in_scale_factor
	)
	: _text_manager(in_text_manager)
	, _behaviour(in_behaviour)
	, _text_run(std::move(in_text_run))
	, _increase_ui_scale_by_avaliable_width_factor(in_enable_scale)
	, _scale_width_low_threashhold(in_scale_threashold)
	, _scale_factor(in_scale_factor)
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
		const float ui_scale = CalculateLocalUiScale(in_avaliable_size.GetX(), in_ui_scale);
		text_run_raw->SetUIScale(ui_scale);

		result = text_run_raw->GetTextBounds();
	}
	// TODO: deal with protection against zero size desired size for ui components, possibly you just don't draw the ui component?
	return result;
}

void DscUi::UiComponentText::Draw(
	DscRenderResource::Frame& in_frame,
	DscRender::IRenderTarget& in_render_target,
	const float //DSC_DEBUG_ONLY(in_ui_scale)  // we use the scale as set by convert avaliable size
)
{
	in_frame.SetRenderTarget(&in_render_target);

	DscText::TextRun* const text_run_raw = _text_run.get();
	// we dont get to check, as we dont still have the in_avaliable_width around to callculate the scale
	//DSC_ASSERT(in_ui_scale == text_run_raw->GetUIScale(), "invalid state");
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

const float DscUi::UiComponentText::CalculateLocalUiScale(const int32 in_avaliable_width, const float in_ui_scale)
{
	if (_increase_ui_scale_by_avaliable_width_factor && (_scale_width_low_threashhold < in_avaliable_width))
	{
		return in_ui_scale * (1.0f + (static_cast<float>(in_avaliable_width - _scale_width_low_threashhold) * _scale_factor));
	}
	return in_ui_scale;
}

