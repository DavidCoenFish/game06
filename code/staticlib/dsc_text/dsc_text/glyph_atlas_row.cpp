#include "glyph_atlas_row.h"

DscText::GlyphAtlasRow::GlyphAtlasRow(
	const int in_mask_index,
	const int in_height,
	const int in_texture_pos_y
)
	: _mask_index(in_mask_index)
	, _height(in_height)
	, _texture_pos_y(in_texture_pos_y)
	, _texture_highest_x(0)
{
	// Nop
}

void DscText::GlyphAtlasRow::GlyphAtlasRow::IncrementTextureHighestX(const int in_add_x)
{
	_texture_highest_x += in_add_x;
}



