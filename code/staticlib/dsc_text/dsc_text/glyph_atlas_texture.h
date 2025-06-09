#pragma once
#include "dsc_text.h"
#include <dsc_common/dsc_common.h>

struct ID3D12GraphicsCommandList;

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

		GlyphAtlasTexture(DscRender::DrawSystem& in_draw_system, const int32 in_texture_dimention = 2048);
		~GlyphAtlasTexture();

		std::unique_ptr<Glyph> AddIcon(const int32 in_width, const int32 in_height, const uint8_t* const in_data_4b);
		std::unique_ptr<Glyph> AddGlyph(const int32 in_width, const int32 in_height, 
			const int32 in_bearing_x,
			const int32 in_bearing_y,
			const uint8_t* const in_data_1b);

		void ClearAllIconUsage();
		void ClearAllGlyphUsage();

		// get reference to the backing texture
		std::shared_ptr<DscRender::HeapWrapperItem> GetHeapWrapperItem() const;

		void UploadTexture(
			DscRender::DrawSystem* const in_draw_system,
			ID3D12GraphicsCommandList* const in_command_list
			);

	private:
		const int32 _texture_dimention;
		std::unique_ptr<DscRenderResource::ShaderResourcePartialUpload> _texture = {};

		std::vector<std::unique_ptr<GlyphAtlasRow>> _array_glyph_row;
		// No, you would still need to visit _array_glyph_row_full to get the highest line in each 
		std::vector<std::unique_ptr<GlyphAtlasRow>> _array_glyph_row_full;
		// working up the texture, this is the highest hight row of pixels a text glyph touches
		int32 _text_highest_pos_y[4] = {0,0,0,0};

		std::vector<std::unique_ptr<GlyphAtlasRow>> _array_icon_row;
		std::vector<std::unique_ptr<GlyphAtlasRow>> _array_icon_row_full;
		// working down the texture, this is the lowest row of pixels touched by an icon texture data
		int32 _icon_lowest_pos_y;

	};
}

