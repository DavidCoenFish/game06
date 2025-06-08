#include "vector_int2.h"

const DscCommon::VectorInt2 DscCommon::VectorInt2::s_zero(0, 0);

DscCommon::VectorInt2::VectorInt2(
	const int32 in_x,
	const int32 in_y
	) : _data{ in_x, in_y }
{
	return;
}

int32& DscCommon::VectorInt2::operator[](const int in_index)
{
	if ((0 <= in_index) && (in_index < Index::Count))
	{
		return _data[in_index];
	}
	static int32 s_dummy = 0;
	return s_dummy;
}

const int32 DscCommon::VectorInt2::operator[](const int in_index) const
{
	if ((0 <= in_index) && (in_index < Index::Count))
	{
		return _data[in_index];
	}
	return 0;
}

const int32 DscCommon::VectorInt2::GetX() const
{
	return _data[Index::X];
}

const int32 DscCommon::VectorInt2::GetY() const
{
	return _data[Index::Y];
}

void DscCommon::VectorInt2::Set(
	const int32 in_x,
	const int32 in_y
	)
{
	_data[Index::X] = in_x;
	_data[Index::Y] = in_y;
	return;
}

const bool DscCommon::VectorInt2::operator==(const VectorInt2& in_rhs) const
{
	for (int index = 0; index < Index::Count;++ index)
	{
		if (_data[index] != in_rhs._data[index])
		{
			return false;
		}
	}
	return true;
}

const bool DscCommon::VectorInt2::operator!=(const VectorInt2& in_rhs) const
{
	return !operator==(in_rhs);
}


DscCommon::VectorInt2& DscCommon::VectorInt2::operator=(const VectorInt2& in_rhs)
{
	for (int index = 0; index < Index::Count; ++index)
	{
		_data[index] = in_rhs._data[index];
	}
	return (*this);
}
