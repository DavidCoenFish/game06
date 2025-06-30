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

	private:
		DagGroupUiComponent _ui_component_group;

	};
}