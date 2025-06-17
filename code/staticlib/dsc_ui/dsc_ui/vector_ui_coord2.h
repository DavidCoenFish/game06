#pragma once
#include "dsc_ui.h"
#include "ui_coord.h"
#include <dsc_common\vector_2.h>
#include <dsc_common\vector_int2.h>

namespace DscUi
{
	//typedef DscCommon::Vector2<UiCoord> VectorUiCoord2;

	//const DscCommon::VectorInt2 EvalueUICoord(const VectorUiCoord2& in_ui_coord, const DscCommon::VectorInt2& in_parent_size);

	// if we end up with wanting another vector like class with object members, make a VectorRef2?
	class VectorUiCoord2
	{
	public:
		explicit VectorUiCoord2(
			const UiCoord& in_x = UiCoord(0, 0.0f),
			const UiCoord& in_y = UiCoord(0, 0.0f)
			);

		const DscCommon::VectorInt2 EvalueUICoord(const DscCommon::VectorInt2& in_parent_size) const;

		UiCoord& operator[](const int in_index);
		const UiCoord operator[](const int in_index) const;
		const UiCoord GetX() const;
		const UiCoord GetY() const;
		void Set(
			const UiCoord& in_x,
			const UiCoord& in_y
		);
		const bool operator==(const VectorUiCoord2& in_rhs) const;
		const bool operator!=(const VectorUiCoord2& in_rhs) const;
		VectorUiCoord2& operator=(const VectorUiCoord2& in_rhs);

	private:
		enum Index
		{
			X = 0,
			Y,
			Count
		};

	private:
		UiCoord _data[Index::Count];
	};


}