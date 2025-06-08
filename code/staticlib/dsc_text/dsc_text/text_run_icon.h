#pragma once
#include "dsc_text.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_float4.h>
#include <dsc_text/i_text_run.h>

namespace DscCommon
{
	class VectorInt2;
}

namespace DscText
{
	class TextPreVertex;

	class TextRunIcon : public ITextRun
	{
	public:
		explicit TextRunIcon(
			const int in_icon_id = 0,
			const DscCommon::VectorFloat4& in_colour_tint = DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 1.0f),
			const float in_new_line_gap_ratio = 0.0f
		);

	private:
		virtual void BuildPreVertexData(
			TextPreVertex& in_out_pre_vertex_data,
			DscCommon::VectorInt2& in_out_cursor,
			const bool in_width_limit_enabled,
			const int in_width_limit,
			const float in_ui_scale
		) const override;

	private:
		int _icon_id;
		float _new_line_gap_ratio;
		DscCommon::VectorFloat4 _colour_tint;

	};
}
