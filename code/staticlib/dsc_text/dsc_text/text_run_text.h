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
			const int in_font_size = 0,
			const float in_new_line_gap_ratio = 0.0f,
			const DscCommon::VectorFloat4& in_colour = DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 1.0f)
		);

	private:
		virtual void BuildPreVertexData(
			TextPreVertex& in_out_pre_vertex_data,
			DscCommon::VectorInt2& in_out_cursor,
			const bool in_width_limit_enabled,
			const int in_width_limit,
			const float in_ui_scale
		) override;

	private:
		const std::string _string_utf8 = {};
		GlyphCollectionText* const _font = nullptr;
		const TextLocale* const _locale_token = nullptr;
		const int _font_size = 0;
		const float _new_line_gap_ratio = 0;
		const DscCommon::VectorFloat4 _colour;
	};
}
