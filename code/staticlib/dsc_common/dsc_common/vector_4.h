#pragma once
#include "dsc_common.h"

namespace DscCommon
{
	template <typename TYPE>
	class Vector4
	{
	public:
		static const Vector4 s_zero;

		explicit Vector4(
			const TYPE in_x = 0,
			const TYPE in_y = 0,
			const TYPE in_z = 0,
			const TYPE in_w = 0
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
			static int32 s_dummy = 0;
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
		TYPE _data[Index::Count];
	};
} //namespace DscCommon
