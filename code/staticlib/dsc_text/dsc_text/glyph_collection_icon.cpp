#include "glyph_collection_icon.h"

#include "glyph.h"
#include "glyph_atlas_texture.h"
#include "text_pre_vertex.h"

const int32 DscText::GlyphCollectionIcon::AddIcon(
	GlyphAtlasTexture& in_texture,
	const DscCommon::VectorInt2& in_size,
	const uint8_t* const in_buffer
	)
{
	const int32 index = static_cast<int32>(_map_icon_cell.size());
	_map_icon_cell.insert(std::make_pair(index, in_texture.AddIcon(in_size, in_buffer)));
	return index;
}

void DscText::GlyphCollectionIcon::BuildPreVertexData(
	TextPreVertex& in_out_text_pre_vertex,
	int32& in_out_cursor,
	const int32 in_icon_id,
	const bool in_width_limit_enabled,
	const int32 in_width_limit,
	const float in_ui_scale,
	const int32 in_colour_tint,
	const int32 in_line_gap_pixels,
	const int32 in_line_minimum_height
	)
{
	auto found = _map_icon_cell.find(in_icon_id);
	if (found == _map_icon_cell.end())
	{
		return;
	}
	auto& cell = *(found->second);
	const DscCommon::VectorInt2& cell_width_height = cell.GetWidthHeightRef();
	const int width = static_cast<int>(round(static_cast<float>(cell_width_height.GetX()) * in_ui_scale));
	if ((true == in_width_limit_enabled) &&
		(in_width_limit < (in_out_cursor + width)))
	{
		in_out_text_pre_vertex.StartNewLine(in_out_cursor, in_line_gap_pixels);
	}
	in_out_text_pre_vertex.AddPreVertexScale(
		cell,
		in_out_cursor,
		0,
		in_line_minimum_height,
		in_colour_tint,
		in_ui_scale
	);

	in_out_cursor += cell_width_height[0];

	return;

}


