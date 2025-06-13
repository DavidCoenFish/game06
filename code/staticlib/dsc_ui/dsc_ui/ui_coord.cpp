#include "ui_coord.h"

DscUi::UiCoord::UiCoord(
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

const bool DscUi::UiCoord::operator==(const UiCoord& in_rhs) const
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
const bool DscUi::UiCoord::operator!=(const UiCoord& in_rhs) const
{
	return !operator==(in_rhs);
}
DscUi::UiCoord& DscUi::UiCoord::operator=(const UiCoord& in_rhs)
{
	_pixels = in_rhs._pixels;
	_ratio = in_rhs._ratio;
	_method = in_rhs._method;
	return (*this);
}

const int32 DscUi::UiCoord::Evaluate(const int32 in_parent_primary, const int32 in_parent_secondary) const
{
	switch (_method)
	{
	default:
		break;
	case TMethod::TMin:
		return static_cast<int32>(round(static_cast<float>(std::min(in_parent_primary, in_parent_secondary)) * _ratio)) + _pixels;
	case TMethod::TMax:
		return static_cast<int32>(round(static_cast<float>(std::max(in_parent_primary, in_parent_secondary)) * _ratio)) + _pixels;
	}
	return static_cast<int32>(round(static_cast<float>(in_parent_primary) * _ratio)) + _pixels;
}

