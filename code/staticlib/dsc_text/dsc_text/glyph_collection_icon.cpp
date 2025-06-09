#include "glyph_collection_icon.h"

#include "glyph.h"
#include "glyph_atlas_texture.h"


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

