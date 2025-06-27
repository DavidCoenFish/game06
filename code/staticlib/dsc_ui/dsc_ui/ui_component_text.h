#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include "ui_enum.h"

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

		virtual const DscCommon::VectorInt2 ConvertAvaliableSizeToDesiredSize(const DscCommon::VectorInt2& in_avaliable_size, const float in_ui_scale) override;

		virtual void Draw(
			DscRenderResource::Frame& in_frame,
			DscRender::IRenderTarget& in_render_target,
			const float in_ui_draw_scale
		) override;

		virtual void SetClearColour(const DscCommon::VectorFloat4& in_colour) override;

		virtual void SetParentChildIndex(const int32 in_parent_child_index) override;

		virtual void SetNode(
			DscDag::NodeToken in_parent_child_index,
			DscDag::NodeToken in_clear_colour_node,
			DscDag::NodeToken in_manual_scroll_x,
			DscDag::NodeToken in_manual_scroll_y
		) override;

	private:
		DscText::TextManager& _text_manager;
		TUiComponentBehaviour _behaviour = {};

		std::unique_ptr<DscText::TextRun> _text_run = {};

		DscDag::NodeToken _parent_child_index = {};
		DscDag::NodeToken _clear_colour_node = {};
		DscDag::NodeToken _manual_scroll_x = {};
		DscDag::NodeToken _manual_scroll_y = {};

	};
}