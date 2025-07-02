#pragma once
#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_float4.h>
#include <dsc_common/vector_int2.h>

typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_FaceRec_* FT_Face;
typedef struct FT_GlyphSlotRec_* FT_GlyphSlot;
struct hb_font_t;
struct hb_buffer_t;
struct hb_feature_t;
typedef uint32_t hb_codepoint_t;

namespace DscCommon
{
	class FileSystem;
}

namespace DscText
{
	class Glyph;
	class GlyphAtlasRow;
	class GlyphAtlasTexture;
	class TextLocale;
	class TextPreVertex;

	// rename as Font? TextFont?
	//no, don't reset, icons have longer lifespan, move GlyphCell ownership to TextFont
	//rather than a reset method, just destroy and recreate, but then what about icon.
	// motivation, want a way to reset all text glyph usage on locale change
	class GlyphCollectionText
	{
	public:
		typedef std::map<uint32_t, std::unique_ptr<Glyph>> TMapCodepointGlyph;

		GlyphCollectionText(
			FT_Library in_library,
			GlyphAtlasTexture* in_texture,
			DscCommon::FileSystem& in_file_system,
			const std::string& in_font_file_path
		);
		~GlyphCollectionText();

		void ClearAllGlyphUsage();

		void BuildPreVertexData(
			TextPreVertex& in_out_text_pre_vertex,
			int32& in_out_cursor, // allow multiple fonts to append pre vertex data
			const std::string& in_string_utf8,
			const TextLocale* const in_locale_token,
			const int in_font_size,
			const bool in_width_limit_enabled = false,
			const int in_width_limit = 0,
			const int32 in_colour = 0xff000000,
			const int32 in_line_minimum_height = 0,
			const int32 in_line_minimum_depth = 0,
			const int32 in_line_gap_pixel = 0,
			const int32 in_base_line_offset = 0
		);

	private:
		struct FontFace
		{
			FT_Face _face = {};
			hb_font_t* _harf_buzz_font = {};
			TMapCodepointGlyph _map_codepoint_glyph = {};
		};

		FontFace* const FindMapCodepointGlyph(const int in_font_size);
		//void SetScale(const int32 in_glyph_size);
		void ShapeText(
			DscText::TextPreVertex& in_out_text_pre_vertex,
			int32& in_out_cursor,
			const std::string& in_string_utf8,
			hb_buffer_t* in_buffer,
			FontFace& in_font_face,
			const bool in_width_limit_enabled,
			const int32 in_width_limit,
			const int32 in_line_minimum_height,
			const int32 in_line_minimum_depth,
			const int32 in_colour,
			const int32 in_line_gap_pixel,
			const int32 in_base_line_offset
		);
		Glyph* FindCell(
			hb_codepoint_t in_codepoint,
			TMapCodepointGlyph& in_out_map_glyph_cell
		);
		Glyph* MakeGlyph(
			hb_codepoint_t in_codepoint,
			FT_GlyphSlot in_slot,
			TMapCodepointGlyph& in_out_map_glyph_cell
		);

	private:
		FT_Library _library;

		// For each font size, have a map of codepoints to glyph
		std::map<uint32_t, std::unique_ptr<FontFace>> _map_size_font_face = {};

		GlyphAtlasTexture* _texture = {};
		// WARNING: font file data needs to be kept around, u.Tag references memory in the file?
		std::vector<uint8> _font_data = {};

		std::vector<hb_feature_t> _features = {};

	};
}

