#pragma once
#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_float4.h>
#include <dsc_common/vector_int2.h>

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
	class Glyph;
	class GlyphAtlasRow;

	//no, don't reset, icons have longer lifespan, move GlyphCell ownership to TextFont
	//rather than a reset method, just destroy and recreate, but then what about icon.
	// motivation, want a way to reset all text glyph usage on locale change
	class Glyph
	{
	public:
		Glyph() = delete;
		Glyph& operator=(const Glyph&) = delete;
		Glyph(const Glyph&) = delete;

		Glyph(
			const VectorInt2& in_width_height,
			const VectorInt2& in_bearing,
			const VectorFloat4& in_uv,
			const int in_mask
		);

		const DscCommon::VectorInt2& GetWidthHeightRef() const { return _width_height; }
		const DscCommon::VectorInt2 GetWidthHeight(const float in_ui_scale) const;
		const DscCommon::VectorInt2& GetBearingRef() const { return _bearing; }
		const DscCommon::VectorInt2 GetBearing(const float in_ui_scale) const;
		const DscCommon::VectorFloat4& GetUVRef() const { return _uv; }
		int GetMask() const { return _mask; }

	private:
		DscCommon::VectorInt2 _width_height;
		DscCommon::VectorInt2 _bearing;
		DscCommon::VectorFloat4 _uv;
		int _mask;

	};
}

