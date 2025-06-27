#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include <dsc_common\vector_float4.h>

namespace DscUi
{
	class UiComponentFill : public IUiComponent
	{
	public:
		UiComponentFill();

	private:
		virtual const bool IsAllowedToBeTopLevelUiComponent() override
		{
			return true;
		}

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
		DscDag::NodeToken _parent_child_index;
		DscDag::NodeToken _clear_colour_node;

	};
}