#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include <dsc_common\vector_float4.h>

namespace DscUi
{
	class UiComponentFill : public IUiComponent
	{
	public:
		UiComponentFill(
			const int32 in_parent_child_index,
			const DscCommon::VectorFloat4& in_fill_colour
			);

	private:
		virtual void Draw(
			DscRenderResource::Frame& in_frame,
			const DscCommon::VectorInt2& in_target_size//, 
			//const std::vector<DscRender::IRenderTarget*>& in_child_render_target_array
		) override;

		virtual const DscCommon::VectorFloat4& GetClearColour() const override;

		virtual const int32 GetParentChildIndex() const override; // what child index are we of out parent

	private:
		int32 _parent_child_index = 0;

		// if this is to be changed, move GetClearColour to a DagNode
		DscCommon::VectorFloat4 _fill_colour;
	};
}