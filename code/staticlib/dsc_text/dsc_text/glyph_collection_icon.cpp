#include "glyph_collection_icon.h"

#include "glyph.h"
#include "glyph_atlas_texture.h"
#include "text_pre_vertex.h"

void DscText::GlyphCollectionIcon::AddIcon(
	GlyphAtlasTexture& in_texture,
	const int32 in_icon_id,
	const int32 in_width,
	const int32 in_height,
	const uint8_t* const in_buffer
	)
{
	if (_map_icon_cell.find(in_icon_id) != _map_icon_cell.end())
	{
		DSC_ASSERT_ALWAYS("icon id duplication");
		return;
	}

	_map_icon_cell.insert(std::make_pair(in_icon_id, in_texture.AddIcon(in_width, in_height, in_buffer)));
}

void DscText::GlyphCollectionIcon::BuildPreVertexData(
	TextPreVertex& in_out_text_pre_vertex,
	DscCommon::VectorInt2& in_out_cursor, // Allow multiple fonts to append pre vertex data
	const int32 in_icon_id,
	const float in_new_line_gap_ratio,
	const bool in_width_limit_enabled,
	const int32 in_width_limit,
	const float in_ui_scale,
	const DscCommon::VectorFloat4& in_colour_tint
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
		(in_width_limit < (in_out_cursor.GetX() + width)))
	{
		in_out_text_pre_vertex.StartNewLine(in_out_cursor);
	}
	in_out_text_pre_vertex.AddPreVertexScale(
		cell,
		in_out_cursor[0],
		in_out_cursor[1],
		in_new_line_gap_ratio,
		in_colour_tint,
		in_ui_scale
	);

	in_out_cursor[0] += cell_width_height[0];

	return;

}


