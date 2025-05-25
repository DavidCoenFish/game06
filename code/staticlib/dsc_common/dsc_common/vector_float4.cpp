#include "vector_float4.h"

const DscCommon::VectorFloat4 DscCommon::VectorFloat4::s_zero(0.0f, 0.0f, 0.0f, 0.0f);
const DscCommon::VectorFloat4 DscCommon::VectorFloat4::s_white(1.0f, 1.0f, 1.0f, 1.0f);
const DscCommon::VectorFloat4 DscCommon::VectorFloat4::s_black(0.0f, 0.0f, 0.0f, 1.0f);
const DscCommon::VectorFloat4 DscCommon::VectorFloat4::s_render_coordinates_full(-1.0f, -1.0f, 1.0f, 1.0f);

DscCommon::VectorFloat4::VectorFloat4(
	const float in_x,
	const float in_y,
	const float in_z,
	const float in_w
	) : _data{ in_x, in_y, in_z, in_w}
{
	return;
}

float& DscCommon::VectorFloat4::operator[](const int in_index)
{
	if ((0 <= in_index) && (in_index < Index::Count))
	{
		return _data[in_index];
	}
	static float s_dummy = 0;
	return s_dummy;
}

const float DscCommon::VectorFloat4::operator[](const int in_index) const
{
	if ((0 <= in_index) && (in_index < Index::Count))
	{
		return _data[in_index];
	}
	return 0;
}

const float DscCommon::VectorFloat4::GetX() const
{
	return _data[Index::X];
}

const float DscCommon::VectorFloat4::GetY() const
{
	return _data[Index::Y];
}

const float DscCommon::VectorFloat4::GetZ() const
{
	return _data[Index::Z];
}

const float DscCommon::VectorFloat4::GetW() const
{
	return _data[Index::W];
}

void DscCommon::VectorFloat4::Set(
	const float in_x,
	const float in_y,
	const float in_z,
	const float in_w
	)
{
	_data[Index::X] = in_x;
	_data[Index::Y] = in_y;
	_data[Index::Z] = in_z;
	_data[Index::W] = in_w;
	return;
}

const bool DscCommon::VectorFloat4::operator==(const VectorFloat4& in_rhs) const
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

const bool DscCommon::VectorFloat4::operator!=(const VectorFloat4& in_rhs) const
{
	return !operator==(in_rhs);
}


DscCommon::VectorFloat4& DscCommon::VectorFloat4::operator=(const VectorFloat4& in_rhs)
{
	for (int index = 0; index < Index::Count; ++index)
	{
		_data[index] = in_rhs._data[index];
	}
	return (*this);
}
