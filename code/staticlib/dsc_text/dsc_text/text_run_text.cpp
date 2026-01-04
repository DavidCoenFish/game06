#include "text_run_text.h"
#include "glyph_collection_text.h"
#include <dsc_common\math.h>

DscText::TextRunText::TextRunText(
	const std::string& in_string_utf8,
	GlyphCollectionText* const in_font,
	const TextLocale* const in_locale_token,
	const int32 in_font_size,
	const int32 in_colour,
	const int32 in_line_minimum_height,
	const int32 in_line_minimum_depth,
	const int32 in_base_line_offset
)
	: _string_utf8(in_string_utf8)
	, _font(in_font)
	, _locale_token(in_locale_token)
	, _font_size(in_font_size)
	, _colour(in_colour)
	, _line_minimum_height(in_line_minimum_height)
	, _line_minimum_depth(in_line_minimum_depth)
	, _base_line_offset(in_base_line_offset)
{
	//nop
}

void DscText::TextRunText::BuildPreVertexData(
	TextPreVertex& in_out_pre_vertex_data,
	int32& in_out_cursor,
	const bool in_width_limit_enabled,
	const int32 in_width_limit,
	const int32 in_new_line_gap_pixels,
	const float in_ui_scale
)
{
	const int32 scale_font_size = DscCommon::Math::ScaleInt(_font_size, in_ui_scale);
	const int32 scale_line_minimum_height = DscCommon::Math::ScaleInt(_line_minimum_height, in_ui_scale);
	const int32 scale_line_minimum_depth = DscCommon::Math::ScaleInt(_line_minimum_depth, in_ui_scale);
	const int32 scale_new_line_gap_pixels = DscCommon::Math::ScaleInt(in_new_line_gap_pixels, in_ui_scale);
	const int32 scale_base_line_offset = DscCommon::Math::ScaleInt(_base_line_offset, in_ui_scale);
	_font->BuildPreVertexData(
		in_out_pre_vertex_data,
		in_out_cursor,
		_string_utf8,
		_locale_token,
		scale_font_size,
		in_width_limit_enabled,
		in_width_limit,
		_colour,
		scale_line_minimum_height,
		scale_line_minimum_depth,
		scale_new_line_gap_pixels,
		scale_base_line_offset
	);
}

