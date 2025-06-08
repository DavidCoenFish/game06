#pragma once
#include "dsc_text.h"
#include <dsc_common/dsc_common.h>

namespace DscRender
{
	class DrawSystem;
	class HeapWrapperItem;
}

namespace DscRenderResource
{
	class ShaderResourcePartialUpload;
}

namespace DscText
{
	class Glyph;
	class GlyphAtlasRow;

	//no, don't reset, icons have longer lifespan, move GlyphCell ownership to TextFont
	//rather than a reset method, just destroy and recreate, but then what about icon.
	// motivation, want a way to reset all text glyph usage on locale change
	class GlyphAtlasTexture
	{
	public:
		GlyphAtlasTexture() = delete;
		GlyphAtlasTexture& operator=(const GlyphAtlasTexture&) = delete;
		GlyphAtlasTexture(const GlyphAtlasTexture&) = delete;

		GlyphAtlasTexture(DscRender::DrawSystem& drawSystem);
		~GlyphAtlasTexture();

		std::unique_ptr<Glyph> AddIcon(const int32 in_width, const int32 in_height, const std::vector<uint8>& in_data_4b);
		std::unique_ptr<Glyph> AddGlyph(const int32 in_width, const int32 in_height, const std::vector<uint8>& in_data_1b);

		void ClearAllIconUsage();
		void ClearAllGlyphUsage();

		// get reference to the backing texture
		std::shared_ptr<DscRender::HeapWrapperItem> GetHeapWrapperItem() const;

		//void UploadTexture(DrawSystem);

	private:
		std::unique_ptr<DscRenderResource::ShaderResourcePartialUpload> _texture = {};

		std::vector<std::unique_ptr<GlyphAtlasRow>> _array_glyph_row;
		// No, you would still need to visit _array_glyph_row_full to get the highest line in each 
		std::vector<std::unique_ptr<GlyphAtlasRow>> _array_glyph_row_full;
		int _highest_pos_y[4];

		std::vector<std::unique_ptr<GlyphAtlasRow>> _array_icon_row;
		std::vector<std::unique_ptr<GlyphAtlasRow>> _array_icon_row_full;
		int _icon_max_pos_y;

	};
}

