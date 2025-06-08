#include "vector_float2.h"

const DscCommon::VectorFloat2 DscCommon::VectorFloat2::s_zero(0, 0);

DscCommon::VectorFloat2::VectorFloat2(
	const float in_x,
	const float in_y
	) : _data{ in_x, in_y }
{
	return;
}

float& DscCommon::VectorFloat2::operator[](const int in_index)
{
	if ((0 <= in_index) && (in_index < Index::Count))
	{
		return _data[in_index];
	}
	static float s_dummy = 0;
	return s_dummy;
}

const float DscCommon::VectorFloat2::operator[](const int in_index) const
{
	if ((0 <= in_index) && (in_index < Index::Count))
	{
		return _data[in_index];
	}
	return 0;
}

const float DscCommon::VectorFloat2::GetX() const
{
	return _data[Index::X];
}

const float DscCommon::VectorFloat2::GetY() const
{
	return _data[Index::Y];
}

void DscCommon::VectorFloat2::Set(
	const float in_x,
	const float in_y
	)
{
	_data[Index::X] = in_x;
	_data[Index::Y] = in_y;
	return;
}

const bool DscCommon::VectorFloat2::operator==(const VectorFloat2& in_rhs) const
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

const bool DscCommon::VectorFloat2::operator!=(const VectorFloat2& in_rhs) const
{
	return !operator==(in_rhs);
}


DscCommon::VectorFloat2& DscCommon::VectorFloat2::operator=(const VectorFloat2& in_rhs)
{
	for (int index = 0; index < Index::Count; ++index)
	{
		_data[index] = in_rhs._data[index];
	}
	return (*this);
}
