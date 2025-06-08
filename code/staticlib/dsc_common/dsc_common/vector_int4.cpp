#include "vector_int4.h"

const DscCommon::VectorInt4 DscCommon::VectorInt4::s_zero(0, 0, 0, 0);

DscCommon::VectorInt4::VectorInt4(
	const int32 in_x,
	const int32 in_y,
	const int32 in_z,
	const int32 in_w
	) : _data{ in_x, in_y, in_z, in_w}
{
	return;
}

int32& DscCommon::VectorInt4::operator[](const int in_index)
{
	if ((0 <= in_index) && (in_index < Index::Count))
	{
		return _data[in_index];
	}
	static int32 s_dummy = 0;
	return s_dummy;
}

const int32 DscCommon::VectorInt4::operator[](const int in_index) const
{
	if ((0 <= in_index) && (in_index < Index::Count))
	{
		return _data[in_index];
	}
	return 0;
}

const int32 DscCommon::VectorInt4::GetX() const
{
	return _data[Index::X];
}

const int32 DscCommon::VectorInt4::GetY() const
{
	return _data[Index::Y];
}

const int32 DscCommon::VectorInt4::GetZ() const
{
	return _data[Index::Z];
}

const int32 DscCommon::VectorInt4::GetW() const
{
	return _data[Index::W];
}

void DscCommon::VectorInt4::Set(
	const int32 in_x,
	const int32 in_y,
	const int32 in_z,
	const int32 in_w
	)
{
	_data[Index::X] = in_x;
	_data[Index::Y] = in_y;
	_data[Index::Z] = in_z;
	_data[Index::W] = in_w;
	return;
}

const bool DscCommon::VectorInt4::operator==(const VectorInt4& in_rhs) const
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

const bool DscCommon::VectorInt4::operator!=(const VectorInt4& in_rhs) const
{
	return !operator==(in_rhs);
}


DscCommon::VectorInt4& DscCommon::VectorInt4::operator=(const VectorInt4& in_rhs)
{
	for (int index = 0; index < Index::Count; ++index)
	{
		_data[index] = in_rhs._data[index];
	}
	return (*this);
}
