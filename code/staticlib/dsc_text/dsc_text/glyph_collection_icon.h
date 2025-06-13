#pragma once
#include "dsc_text.h"

#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_float4.h>

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
}

namespace DscText
{
	class Glyph;
	class GlyphAtlasTexture;
	class TextPreVertex;

	class GlyphCollectionIcon
	{
	public:
		void AddIcon(
			GlyphAtlasTexture& in_texture,
			const int32 in_icon_id,
			const int32 in_width,
			const int32 in_height,
			const uint8_t* const in_buffer
		);

		void BuildPreVertexData(
			TextPreVertex& in_out_text_pre_vertex,
			int32& in_out_cursor, // Allow multiple fonts to append pre vertex data
			const int32 in_icon_id,
			const bool in_width_limit_enabled = false,
			const int32 in_width_limit = 0,
			const float in_ui_scale = 1.0f,
			const int32 in_colour = 0xffffffff,
			const int32 in_line_gap_pixels = 0,
			const int32 in_line_minimum_height = 0
		);


	private:
		std::map<int, std::unique_ptr<Glyph>> _map_icon_cell;

	};
}

