#pragma once
#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_float4.h>
#include <dsc_common/vector_int2.h>

typedef struct FT_LibraryRec_* FT_Library;

namespace DscCommon
{
	class FileSystem;
}

namespace DscText
{
	class Glyph;
	class GlyphAtlasRow;
	class TextLocale;
	class TextPreVertex;

	//no, don't reset, icons have longer lifespan, move GlyphCell ownership to TextFont
	//rather than a reset method, just destroy and recreate, but then what about icon.
	// motivation, want a way to reset all text glyph usage on locale change
	class GlyphCollectionText
	{
	public:
		GlyphCollectionText(
			FT_Library in_library,
			DscCommon::FileSystem& fileSystem,
			const std::string& in_font_file_path
		);
		~GlyphCollectionText();

		void BuildPreVertexData(
			TextPreVertex& in_out_text_pre_vertex,
			DscCommon::VectorInt2& in_out_cursor, // allow multiple fonts to append pre vertex data
			const std::string& in_string_utf8,
			const TextLocale* const in_locale_token,
			const int in_font_size,
			const bool in_width_limit_enabled = false,
			const int in_width_limit = 0,
			const int in_new_line_height = 0,
			const DscCommon::VectorFloat4& in_colour = DscCommon::VectorFloat4(0.0f, 0.0f, 0.0f, 1.0f)
		);

		/// Draw a glyph to a pixel blob, made to match the target size
		void DrawToPixels(
			std::vector<uint8_t>& out_data,
			const int in_target_width,
			const int in_target_height,
			const std::string& in_string_utf8,
			const TextLocale* const in_locale_token,
			const int in_font_size,
			const int in_offset_x = 0,
			const int in_offset_y = 0
		);
	};
}

