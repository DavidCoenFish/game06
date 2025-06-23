#pragma once
#include "dsc_ui.h"

namespace DscUi
{
	class UiCoord
	{
	public:
		enum class TMethod
		{
			TPrimaryPoroportinal = 0, //default, ((in_parent_primary * _ratio) + _pixels)
			TMin, //((std::min(in_parent_primary, in_parent_secondary)* _ratio) + _pixels)
			TMax, //((std::max(in_parent_primary, in_parent_secondary)* _ratio) + _pixels)
		};
		UiCoord() {}

		explicit UiCoord(
			const int32 in_pixels,
			const float in_ratio,
			const TMethod in_method = TMethod::TPrimaryPoroportinal
		);

		const bool operator==(const UiCoord& in_rhs) const;
		const bool operator!=(const UiCoord& in_rhs) const;
		UiCoord& operator=(const UiCoord& in_rhs);

		/// if this is a horizontal ui cord, then the primary input is the parents horizontal value, and vertical the scondary
		/// if this is a vertical ui cord, then the primary input is the parents vertical value, and horizontal the scondary
		const int32 Evaluate(const int32 in_parent_primary, const int32 in_parent_secondary) const;

	private:
		int32 _pixels = 0;
		float _ratio = 0.0f;
		TMethod _method = {};
	};
}