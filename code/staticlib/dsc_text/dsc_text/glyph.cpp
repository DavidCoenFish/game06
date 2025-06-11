#include "glyph.h"

DscText::Glyph::Glyph(
	const DscCommon::VectorInt2& in_width_height,
	const DscCommon::VectorInt2& in_bearing,
	const DscCommon::VectorFloat4& in_uv,
	const int32 in_mask
	)
	: _width_height(in_width_height)
	, _bearing(in_bearing)
	, _uv(in_uv)
	, _mask(in_mask)
{
	// Nop
}

const DscCommon::VectorInt2 DscText::Glyph::GetWidthHeight(const float in_ui_scale) const
{
	return DscCommon::VectorInt2(
		static_cast<int>(round(static_cast<float>(_width_height.GetX()) * in_ui_scale)),
		static_cast<int>(round(static_cast<float>(_width_height.GetY()) * in_ui_scale))
	);
}

const DscCommon::VectorInt2 DscText::Glyph::GetBearing(const float in_ui_scale) const
{
	return DscCommon::VectorInt2(
		static_cast<int>(round(static_cast<float>(_bearing.GetX()) * in_ui_scale)),
		static_cast<int>(round(static_cast<float>(_bearing.GetY()) * in_ui_scale))
	);
}
