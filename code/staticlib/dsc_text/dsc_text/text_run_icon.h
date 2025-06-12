#pragma once
#include "dsc_text.h"
#include <dsc_common/dsc_common.h>
#include <dsc_text/i_text_run.h>

namespace DscCommon
{
	class VectorInt2;
}

namespace DscText
{
	class GlyphCollectionIcon;
	class TextPreVertex;

	class TextRunIcon : public ITextRun
	{
	public:
		TextRunIcon() = delete;
		TextRunIcon& operator=(const TextRunIcon&) = delete;
		TextRunIcon(const TextRunIcon&) = delete;

		explicit TextRunIcon(
			const int32 in_icon_id = 0,
			GlyphCollectionIcon* const in_icon_font = nullptr,
			const int32 in_colour_tint = 0xff000000,
			const int32 in_line_height_minimun = 0
		);

	private:
		virtual void BuildPreVertexData(
			TextPreVertex& in_out_pre_vertex_data,
			int32& in_out_cursor,
			const bool in_width_limit_enabled,
			const int32 in_width_limit,
			const int32 in_new_line_gap_pixels,
			const float in_ui_scale
		) override;

	private:
		const int32 _icon_id = 0;
		GlyphCollectionIcon* const _icon_font = 0;
		const int32 _colour_tint;
		const int32 _line_height_minimun;
	};
}
