#pragma once
#include "dsc_ui_2.h"
#include "coord.h"
#include <dsc_common\vector_2.h>
#include <dsc_common\vector_int2.h>

namespace DscUi2
{
	//typedef DscCommon::Vector2<UiCoord> VectorUiCoord2;

	//const DscCommon::VectorInt2 EvalueUICoord(const VectorUiCoord2& in_ui_coord, const DscCommon::VectorInt2& in_parent_size);

	// if we end up with wanting another vector like class with object members, make a VectorRef2?
	class VectorCoord2
	{
	public:
		VectorCoord2() {};

		explicit VectorCoord2(
			const Coord& in_x,
			const Coord& in_y
			);

		const DscCommon::VectorInt2 EvalueUICoord(const DscCommon::VectorInt2& in_parent_size, const float in_ui_scale = 1.0f) const;

		Coord& operator[](const int in_index);
		const Coord& operator[](const int in_index) const;
		const Coord& GetX() const;
		const Coord& GetY() const;
		void Set(
			const Coord& in_x,
			const Coord& in_y
		);
		const bool operator==(const VectorCoord2& in_rhs) const;
		const bool operator!=(const VectorCoord2& in_rhs) const;
		VectorCoord2& operator=(const VectorCoord2& in_rhs);

	private:
		enum Index
		{
			X = 0,
			Y,
			Count
		};

	private:
		Coord _data[Index::Count] = {};
	};
}