#pragma once
#include "dsc_common.h"

namespace DscCommon
{
	template <typename TYPE>
	class Vector4
	{
	public:
		static const Vector4 s_zero;

		Vector4() {}

		explicit Vector4(
			const TYPE in_x,
			const TYPE in_y,
			const TYPE in_z,
			const TYPE in_w
		) : _data{ in_x, in_y, in_z, in_w }
		{
			return;
		}
		TYPE& operator[](const int in_index)
		{
			if ((0 <= in_index) && (in_index < Index::Count))
			{
				return _data[in_index];
			}
			static TYPE s_dummy = 0;
			return s_dummy;
		}
		const TYPE operator[](const int in_index) const
		{
			if ((0 <= in_index) && (in_index < Index::Count))
			{
				return _data[in_index];
			}
			return 0;
		}
		const TYPE GetX() const
		{
			return _data[Index::X];
		}
		const TYPE GetY() const
		{
			return _data[Index::Y];
		}
		const TYPE GetZ() const
		{
			return _data[Index::Z];
		}
		const TYPE GetW() const
		{
			return _data[Index::W];
		}
		void Set(
			const TYPE in_x,
			const TYPE in_y,
			const TYPE in_z,
			const TYPE in_w
		)
		{
			_data[Index::X] = in_x;
			_data[Index::Y] = in_y;
			_data[Index::Z] = in_z;
			_data[Index::W] = in_w;
			return;
		}

		const bool operator==(const Vector4& in_rhs) const
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

		const bool operator!=(const Vector4& in_rhs) const
		{
			return !operator==(in_rhs);
		}

		Vector4& operator=(const Vector4& in_rhs)
		{
			for (int index = 0; index < Index::Count; ++index)
			{
				_data[index] = in_rhs._data[index];
			}
			return (*this);
		}

	private:
		enum Index
		{
			X = 0,
			Y,
			Z,
			W,
			Count
		};

	private:
		TYPE _data[Index::Count] = {};
	};
} //namespace DscCommon

//template <typename IN_TYPE>
//const bool operator==(const DscCommon::Vector4<IN_TYPE>& in_lhs, const int32 in_rhs)
//{
//	for (int index = 0; index < 4; ++index)
//	{
//		if (in_lhs[index] != in_rhs)
//		{
//			return false;
//		}
//	}
//	return true;
//}
//
//template <typename IN_TYPE>
//const bool operator!=(const DscCommon::Vector4<IN_TYPE>& in_lhs, const int32 in_rhs)
//{
//	return !operator==(in_lhs, in_rhs);
//}
//
//template <typename IN_TYPE>
//const bool operator!=(const int32 in_lhs, const DscCommon::Vector4<IN_TYPE>& in_rhs)
//{
//	return !operator==(in_rhs, in_lhs);
//}
//

template <typename IN_TYPE>
const DscCommon::Vector4<IN_TYPE> operator + (const DscCommon::Vector4<IN_TYPE>& in_lhs, const DscCommon::Vector4<IN_TYPE>& in_rhs)
{
	return DscCommon::Vector4<IN_TYPE>(
		in_lhs[0] + in_rhs[0],
		in_lhs[1] + in_rhs[1],
		in_lhs[2] + in_rhs[2],
		in_lhs[3] + in_rhs[3]
		);
}

template <typename IN_TYPE>
const DscCommon::Vector4<IN_TYPE> operator - (const DscCommon::Vector4<IN_TYPE>& in_lhs, const DscCommon::Vector4<IN_TYPE>& in_rhs)
{
	return DscCommon::Vector4<IN_TYPE>(
		in_lhs[0] - in_rhs[0],
		in_lhs[1] - in_rhs[1],
		in_lhs[2] - in_rhs[2],
		in_lhs[3] - in_rhs[3]
		);
}

template <typename IN_TYPE>
const DscCommon::Vector4<IN_TYPE> operator * (const DscCommon::Vector4<IN_TYPE>& in_lhs, const IN_TYPE in_rhs)
{
	return DscCommon::Vector4<IN_TYPE>(
		in_lhs[0] * in_rhs,
		in_lhs[1] * in_rhs,
		in_lhs[2] * in_rhs,
		in_lhs[3] * in_rhs
		);
}
