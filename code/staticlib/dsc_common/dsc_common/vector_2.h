#pragma once
#include "dsc_common.h"

namespace DscCommon
{
	template <typename TYPE>
	class Vector2
	{
	public:
		static const Vector2 s_zero;

		Vector2() {}

		explicit Vector2(
			const TYPE in_x,
			const TYPE in_y
		) : _data{ in_x, in_y }
		{
			return;
		}

		TYPE& operator[](const int in_index)
		{
			if ((0 <= in_index) && (in_index < Index::Count))
			{
				return _data[in_index];
			}
			static TYPE s_dummy = {};
			return s_dummy;
		}
		const TYPE operator[](const int in_index) const
		{
			if ((0 <= in_index) && (in_index < Index::Count))
			{
				return _data[in_index];
			}
			static TYPE s_dummy = {};
			return s_dummy;
		}
		const TYPE GetX() const
		{
			return _data[Index::X];
		}
		const TYPE GetY() const
		{
			return _data[Index::Y];
		}
		void Set(
			const TYPE in_x,
			const TYPE in_y
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
		TYPE _data[Index::Count] = {};
	};
} //namespace DscCommon
