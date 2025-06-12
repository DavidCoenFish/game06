#include "text_run_icon.h"
#include "glyph_collection_icon.h"

DscText::TextRunIcon::TextRunIcon(
	const int32 in_icon_id,
	GlyphCollectionIcon* const in_icon_font,
	const int32 in_colour_tint,
	const int32 in_line_height_minimun
)
	: _icon_id(in_icon_id)
	, _icon_font(in_icon_font)
	, _colour_tint(in_colour_tint)
	, _line_height_minimun(in_line_height_minimun)
{
	// nop
}

void DscText::TextRunIcon::BuildPreVertexData(
	TextPreVertex& in_out_pre_vertex_data,
	DscCommon::VectorInt2& in_out_cursor,
	const bool in_width_limit_enabled,
	const int32 in_width_limit,
	const int32 in_new_line_gap_pixels,
	const float in_ui_scale
)
{
	_icon_font->BuildPreVertexData(
		in_out_pre_vertex_data,
		in_out_cursor,
		_icon_id,
		in_width_limit_enabled,
		in_width_limit,
		in_ui_scale,
		_colour_tint,
		in_new_line_gap_pixels,
		_line_height_minimun
		);
}
