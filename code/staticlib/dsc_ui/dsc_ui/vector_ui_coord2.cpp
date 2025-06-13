#include "vector_ui_coord2.h"
#include "ui_coord.h"
#include <dsc_common\vector_2.h>

//const DscCommon::Vector2<int32> DscCommon::Vector2<int32>::s_zero(0, 0);

//const DscUi::VectorUiCoord2 DscUi::VectorUiCoord2::s_zero(DscUi::UiCoord(), DscUi::UiCoord());

//const DscUi::VectorUiCoord2 DscUi::VectorUiCoord2::s_zero(
//	DscUi::UiCoord(), 
//	DscUi::UiCoord()
//);

DscUi::VectorUiCoord2::VectorUiCoord2(
	const UiCoord& in_x,
	const UiCoord& in_y
	) : _data{ in_x, in_y }
{
	return;
}

const DscCommon::VectorInt2 DscUi::VectorUiCoord2::EvalueUICoord(const DscCommon::VectorInt2& in_parent_size)
{
	const DscCommon::VectorInt2 result(
		_data[0].Evaluate(in_parent_size.GetX(), in_parent_size.GetY()),
		_data[1].Evaluate(in_parent_size.GetY(), in_parent_size.GetX())
	);
	return result;
}

DscUi::UiCoord& DscUi::VectorUiCoord2::operator[](const int in_index)
{
	if ((0 <= in_index) && (in_index < Index::Count))
	{
		return _data[in_index];
	}
	static UiCoord s_dummy(0, 0.0f);
	return s_dummy;
}
const DscUi::UiCoord DscUi::VectorUiCoord2::operator[](const int in_index) const
{
	if ((0 <= in_index) && (in_index < Index::Count))
	{
		return _data[in_index];
	}
	return UiCoord();
}
const DscUi::UiCoord DscUi::VectorUiCoord2::GetX() const
{
	return _data[Index::X];
}
const DscUi::UiCoord DscUi::VectorUiCoord2::GetY() const
{
	return _data[Index::Y];
}
void DscUi::VectorUiCoord2::Set(
	const UiCoord& in_x,
	const UiCoord& in_y
)
{
	_data[Index::X] = in_x;
	_data[Index::Y] = in_y;
	return;
}

const bool DscUi::VectorUiCoord2::operator==(const VectorUiCoord2& in_rhs) const
{
	for (int index = 0; index < Index::Count; ++index)
	{
		if (_data[index] != in_rhs._data[index])
		{
			return false;
		}
	}
	return true;
}

const bool DscUi::VectorUiCoord2::operator!=(const VectorUiCoord2& in_rhs) const
{
	return !operator==(in_rhs);
}

DscUi::VectorUiCoord2& DscUi::VectorUiCoord2::operator=(const VectorUiCoord2& in_rhs)
{
	for (int index = 0; index < Index::Count; ++index)
	{
		_data[index] = in_rhs._data[index];
	}
	return (*this);
}
