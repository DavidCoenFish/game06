#pragma once
#include "dsc_common.h"

namespace DscCommon
{
	template <typename IN_TYPE>
	class Vector2
	{
	public:
		static const Vector2 s_zero;

		Vector2() {}

		explicit Vector2(
			const IN_TYPE in_x,
			const IN_TYPE in_y
		) : _data{ in_x, in_y }
		{
			return;
		}

		IN_TYPE& operator[](const int in_index)
		{
			if ((0 <= in_index) && (in_index < Index::Count))
			{
				return _data[in_index];
			}
			static IN_TYPE s_dummy = {};
			return s_dummy;
		}
		const IN_TYPE operator[](const int in_index) const
		{
			if ((0 <= in_index) && (in_index < Index::Count))
			{
				return _data[in_index];
			}
			static IN_TYPE s_dummy = {};
			return s_dummy;
		}
		const IN_TYPE GetX() const
		{
			return _data[Index::X];
		}
		const IN_TYPE GetY() const
		{
			return _data[Index::Y];
		}
		void Set(
			const IN_TYPE in_x,
			const IN_TYPE in_y
		)
		{
			_data[Index::X] = in_x;
			_data[Index::Y] = in_y;
			return;
		}

		const bool operator==(const Vector2& in_rhs) const
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

		const bool operator!=(const Vector2& in_rhs) const
		{
			return !operator==(in_rhs);
		}

		Vector2& operator=(const Vector2& in_rhs)
		{
			for (int index = 0; index < Index::Count; ++index)
			{
				_data[index] = in_rhs._data[index];
			}
			return (*this);
		}

		const bool operator<=(const Vector2& in_rhs) const
		{
			for (int index = 0; index < Index::Count; ++index)
			{
				if (_data[index] <= in_rhs._data[index])
				{
					continue;
				}
				else
				{
					return false;
				}

			}
			return true;
		}

	private:
		enum Index
		{
			X = 0,
			Y,
			Count
		};

	private:
		IN_TYPE _data[Index::Count] = {};
	};

	template <typename IN_TYPE>
	const IN_TYPE Dot(const DscCommon::Vector2<IN_TYPE>& in_lhs, const DscCommon::Vector2<IN_TYPE>& in_rhs)
	{
		return (in_lhs[0] * in_rhs[0]) +
			(in_lhs[1] * in_rhs[1]);
	}
} //namespace DscCommon

//template <typename IN_TYPE>
//const bool operator==(const DscCommon::Vector2<IN_TYPE>& in_lhs, const int32 in_rhs)
//{
//	for (int index = 0; index < 2; ++index)
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
//const bool operator!=(const DscCommon::Vector2<IN_TYPE>& in_lhs, const int32 in_rhs)
//{
//	return !operator==(in_lhs, in_rhs);
//}
//
//template <typename IN_TYPE>
//const bool operator!=(const int32 in_lhs, const DscCommon::Vector2<IN_TYPE>& in_rhs)
//{
//	return !operator==(in_rhs, in_lhs);
//}


template <typename IN_TYPE>
const DscCommon::Vector2<IN_TYPE> operator + (const DscCommon::Vector2<IN_TYPE>& in_lhs, const DscCommon::Vector2<IN_TYPE>& in_rhs)
{
	return DscCommon::Vector2<IN_TYPE>(
		in_lhs[0] + in_rhs[0],
		in_lhs[1] + in_rhs[1]
		);
}

template <typename IN_TYPE>
const DscCommon::Vector2<IN_TYPE> operator - (const DscCommon::Vector2<IN_TYPE>& in_lhs, const DscCommon::Vector2<IN_TYPE>& in_rhs)
{
	return DscCommon::Vector2<IN_TYPE>(
		in_lhs[0] - in_rhs[0],
		in_lhs[1] - in_rhs[1]
		);
}

template <typename IN_TYPE>
const DscCommon::Vector2<IN_TYPE> operator * (const DscCommon::Vector2<IN_TYPE>& in_lhs, const IN_TYPE in_rhs)
{
	return DscCommon::Vector2<IN_TYPE>(
		in_lhs[0] * in_rhs,
		in_lhs[1] * in_rhs
		);
}

template <typename IN_TYPE>
const DscCommon::Vector2<IN_TYPE> operator / (const DscCommon::Vector2<IN_TYPE>& in_lhs, const IN_TYPE in_rhs)
{
	return DscCommon::Vector2<IN_TYPE>(
		in_lhs[0] / in_rhs,
		in_lhs[1] / in_rhs
		);
}

