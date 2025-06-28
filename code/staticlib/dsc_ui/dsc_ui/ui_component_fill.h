#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include <dsc_dag\dag_group.h>

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

		virtual void SetNode(const DagGroupUiComponent& in_ui_component_group) override;

	private:
		DagGroupUiComponent _ui_component_group;

	};
}