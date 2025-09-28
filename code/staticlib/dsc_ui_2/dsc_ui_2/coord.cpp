#include "coord.h"
#include <dsc_common\math.h>

DscUi2::Coord::Coord(
	const int32 in_pixels,
	const float in_ratio,
	const TMethod in_method
)
	: _pixels(in_pixels)
	, _ratio(in_ratio)
	, _method(in_method)
{
	// nop
}

const bool DscUi2::Coord::operator==(const Coord& in_rhs) const
{
	if (_pixels != in_rhs._pixels)
	{
		return false;
	}
	if (_ratio != in_rhs._ratio)
	{
		return false;
	}
	if (_method != in_rhs._method)
	{
		return false;
	}
	return true;
}
const bool DscUi2::Coord::operator!=(const Coord& in_rhs) const
{
	return !operator==(in_rhs);
}
DscUi2::Coord& DscUi2::Coord::operator=(const Coord& in_rhs)
{
	_pixels = in_rhs._pixels;
	_ratio = in_rhs._ratio;
	_method = in_rhs._method;
	return (*this);
}

const int32 DscUi2::Coord::Evaluate(const int32 in_parent_primary, const int32 in_parent_secondary, const float in_ui_scale) const
{
	switch (_method)
	{
	default:
		break;
	case TMethod::TSecondaryPoroportinal:
		return DscCommon::Math::ScaleInt(in_parent_secondary, _ratio) + DscCommon::Math::ScaleInt(_pixels, in_ui_scale);
	case TMethod::TMin:
		return DscCommon::Math::ScaleInt(std::min(in_parent_primary, in_parent_secondary), _ratio) + DscCommon::Math::ScaleInt(_pixels, in_ui_scale);
	case TMethod::TMax:
		return DscCommon::Math::ScaleInt(std::max(in_parent_primary, in_parent_secondary), _ratio) + DscCommon::Math::ScaleInt(_pixels, in_ui_scale);
	}

	return DscCommon::Math::ScaleInt(in_parent_primary, _ratio) + DscCommon::Math::ScaleInt(_pixels, in_ui_scale);
}

