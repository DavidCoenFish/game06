#pragma once
#include "dsc_common.h"

namespace DscCommon
{
	class VectorInt4
	{
	public:
		static const VectorInt4 s_zero;

		explicit VectorInt4(
			const int32 in_x = 0,
			const int32 in_y = 0,
			const int32 in_z = 0,
			const int32 in_w = 0
		);
		int32& operator[](const int in_index);
		const int32 operator[](const int in_index) const;
		const int32 GetX() const;
		const int32 GetY() const;
		const int32 GetZ() const;
		const int32 GetW() const;
		void Set(
			const int32 in_x,
			const int32 in_y,
			const int32 in_z,
			const int32 in_w
		);
		const bool operator==(const VectorInt4& in_rhs) const;
		const bool operator!=(const VectorInt4& in_rhs) const;
		VectorInt4& operator=(const VectorInt4& in_rhs);

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
		int32 _data[Index::Count];
	};
} //namespace DscCommon
