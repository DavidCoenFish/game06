#include "text_run_text.h"
#include "glyph_collection_text.h"
#include <dsc_common\math.h>

DscText::TextRunText::TextRunText(
	const std::string& in_string_utf8,
	GlyphCollectionText* const in_font,
	const TextLocale* const in_locale_token,
	const int32 in_font_size,
	const float in_new_line_gap_ratio,
	const int32 in_colour
	)
	: _string_utf8(in_string_utf8)
	, _font(in_font)
	, _locale_token(in_locale_token)
	, _font_size(in_font_size)
	, _new_line_gap_ratio(in_new_line_gap_ratio)
	, _colour(in_colour)
{
	//nop
}

void DscText::TextRunText::BuildPreVertexData(
	TextPreVertex& in_out_pre_vertex_data,
	DscCommon::VectorInt2& in_out_cursor,
	const bool in_width_limit_enabled,
	const int32 in_width_limit,
	const float in_ui_scale
)
{
	_font->BuildPreVertexData(
		in_out_pre_vertex_data,
		in_out_cursor,
		_string_utf8,
		_locale_token,
		DscCommon::Math::ScaleInt(_font_size, in_ui_scale),
		in_width_limit_enabled,
		in_width_limit,
		DscCommon::Math::ScaleInt(_font_size, (1.0f + _new_line_gap_ratio) * in_ui_scale),
		_colour
	);

}

