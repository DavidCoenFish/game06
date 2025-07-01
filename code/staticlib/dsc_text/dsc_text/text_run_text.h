#pragma once
#include "dsc_text.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_float4.h>
#include <dsc_text/i_text_run.h>

namespace DscText
{
	class GlyphCollectionText;
	class TextLocale;
	class TextPreVertex;

	class TextRunText : public ITextRun
	{
	public:
		TextRunText() = delete;
		TextRunText& operator=(const TextRunText&) = delete;
		TextRunText(const TextRunText&) = delete;

		explicit TextRunText(
			const std::string& in_string_utf8 = std::string(""),
			GlyphCollectionText* const in_font = nullptr,
			const TextLocale* const in_locale_token = nullptr,
			const int32 in_font_size = 0,
			const int32 in_colour = 0xff000000,
			const int32 in_line_minimum_height = 0,
			const int32 in_line_minimum_depth = 0
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
		const std::string _string_utf8 = {};
		GlyphCollectionText* const _font = nullptr;
		const TextLocale* const _locale_token = nullptr;
		const int32 _font_size = 0;
		const int32 _colour;
		const int32 _line_minimum_height; // how many pixels above the neutal line we claim to fill
		const int32 _line_minimum_depth; // howm any pixels below the neutal line we claim to fill
	};
}
