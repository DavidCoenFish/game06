#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include <dsc_common\vector_float4.h>

namespace DscUi
{
	class UiComponentFill : public IUiComponent
	{
	public:
		UiComponentFill(const DscCommon::VectorFloat4& in_fill_colour);

	private:
		virtual const DscCommon::VectorFloat4& GetClearColour() const override;

	private:
		// if this is to be changed, move GetClearColour to a DagNode
		DscCommon::VectorFloat4 _fill_colour;
	};
}