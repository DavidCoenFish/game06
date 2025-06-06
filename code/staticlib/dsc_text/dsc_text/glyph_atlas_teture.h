#pragma once
#include <dsc_common/dsc_common.h>

namespace DscRender
{
	class DrawSystem;
	class HeapWrapperItem;
}

namespace DscRenderResource
{
	class ShaderResource;
}

namespace DscText
{
	class GlyphAtlasTexture
	{
	public:
		GlyphAtlasTexture() = delete;
		GlyphAtlasTexture& operator=(const GlyphAtlasTexture&) = delete;
		GlyphAtlasTexture(const GlyphAtlasTexture&) = delete;

		int32 AddIcon(const int32 in_width, const int32 in_height, const std::vector<uint8>& in_data_4b);
		int32 AddGlyph(const int32 in_width, const int32 in_height, const std::vector<uint8>& in_data_1b);

		std::shared_ptr<DscRender::HeapWrapperItem> GetHeapWrapperItem() const;

		//void UploadTexture(DrawSystem);

	private:
		std::unique_ptr<DscRenderResource::ShaderResource> _texture = {};
		// move into ShaderResource?
		//bool _dirty = false;
		//int32 _dirty_height_low = 0;
		//int32 _dirty_height_high = 0;
	};
}

