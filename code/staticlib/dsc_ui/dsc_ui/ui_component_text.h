#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include "ui_enum.h"
#include <dsc_dag\dag_group.h>

namespace DscText
{
	class TextManager;
	class TextRun;
}

namespace DscUi
{
	class UiComponentText : public IUiComponent
	{
	public:
		UiComponentText() = delete;
		UiComponentText& operator=(const UiComponentText&) = delete;
		UiComponentText(const UiComponentText&) = delete;

		UiComponentText(
			DscText::TextManager& in_text_manager,
			std::unique_ptr<DscText::TextRun>&& in_text_run,
			const TUiComponentBehaviour in_behaviour
			);

	private:
		virtual const bool IsAllowedToBeTopLevelUiComponent() override
		{
			return false;
		}

		virtual const DscCommon::VectorInt2 ConvertAvaliableSizeToDesiredSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const DscCommon::VectorInt2& in_avaliable_size, const float in_ui_scale) override;

		virtual void Draw(
			DscRenderResource::Frame& in_frame,
			DscRender::IRenderTarget& in_render_target,
			const float in_ui_draw_scale
		) override;

	private:
		DscText::TextManager& _text_manager;
		TUiComponentBehaviour _behaviour = {};

		std::unique_ptr<DscText::TextRun> _text_run = {};


	};
}