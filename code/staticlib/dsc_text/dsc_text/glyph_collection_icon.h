#pragma once
#include "dsc_text.h"

#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_float4.h>

namespace DscCommon
{
	class VectorInt2;
}

namespace DscText
{
	class TextPreVertex;

	class GlyphCollectionIcon
	{
	public:
		void AddIcon(
			const int in_icon_id,
			const int in_width,
			const int in_height,
			const unsigned char* const in_buffer
		);

		void BuildPreVertexData(
			TextPreVertex& in_out_text_pre_vertex,
			DscCommon::VectorInt2& in_out_cursor, // Allow multiple fonts to append pre vertex data
			const int in_icon_id,
			const float in_new_line_gap_ratio = 0.0f,
			const bool in_width_limit_enabled = false,
			const int in_width_limit = 0,
			const float in_ui_scale = 1.0f,
			const DscCommon::VectorFloat4& in_colour_tint = DscCommon::VectorFloat4(1.0f, 1.0f, 1.0f, 1.0f)
		);


	private:
	};
}

