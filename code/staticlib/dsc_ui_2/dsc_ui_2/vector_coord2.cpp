#include "vector_coord2.h"
#include "coord.h"
#include <dsc_common\vector_2.h>

//const DscCommon::Vector2<int32> DscCommon::Vector2<int32>::s_zero(0, 0);

//const DscUi2::VectorCoord2 DscUi2::VectorCoord2::s_zero(DscUi2::Coord(), DscUi2::Coord());

//const DscUi2::VectorCoord2 DscUi2::VectorCoord2::s_zero(
//	DscUi2::Coord(), 
//	DscUi2::Coord()
//);

DscUi2::VectorCoord2::VectorCoord2(
	const Coord& in_x,
	const Coord& in_y
	) : _data{ in_x, in_y }
{
	return;
}

const DscCommon::VectorInt2 DscUi2::VectorCoord2::EvalueUICoord(const DscCommon::VectorInt2& in_parent_size, const float in_ui_scale) const
{
	const DscCommon::VectorInt2 result(
		_data[0].Evaluate(in_parent_size.GetX(), in_parent_size.GetY(), in_ui_scale),
		_data[1].Evaluate(in_parent_size.GetY(), in_parent_size.GetX(), in_ui_scale)
	);
	return result;
}

DscUi2::Coord& DscUi2::VectorCoord2::operator[](const int in_index)
{
	if ((0 <= in_index) && (in_index < Index::Count))
	{
		return _data[in_index];
	}
	static Coord s_dummy(0, 0.0f);
	return s_dummy;
}
const DscUi2::Coord& DscUi2::VectorCoord2::operator[](const int in_index) const
{
	if ((0 <= in_index) && (in_index < Index::Count))
	{
		return _data[in_index];
	}
	static Coord s_dummy(0, 0.0f);
	return s_dummy;
}
const DscUi2::Coord& DscUi2::VectorCoord2::GetX() const
{
	return _data[Index::X];
}
const DscUi2::Coord& DscUi2::VectorCoord2::GetY() const
{
	return _data[Index::Y];
}
void DscUi2::VectorCoord2::Set(
	const Coord& in_x,
	const Coord& in_y
)
{
	_data[Index::X] = in_x;
	_data[Index::Y] = in_y;
	return;
}

const bool DscUi2::VectorCoord2::operator==(const VectorCoord2& in_rhs) const
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

const bool DscUi2::VectorCoord2::operator!=(const VectorCoord2& in_rhs) const
{
	return !operator==(in_rhs);
}

DscUi2::VectorCoord2& DscUi2::VectorCoord2::operator=(const VectorCoord2& in_rhs)
{
	for (int index = 0; index < Index::Count; ++index)
	{
		_data[index] = in_rhs._data[index];
	}
	return (*this);
}
