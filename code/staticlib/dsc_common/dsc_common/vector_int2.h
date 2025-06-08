#pragma once
#include "dsc_common.h"

namespace DscCommon
{
	class VectorInt2
	{
	public:
		static const VectorInt2 s_zero;

		explicit VectorInt2(
			const int32 in_x = 0,
			const int32 in_y = 0
		);
		int32& operator[](const int in_index);
		const int32 operator[](const int in_index) const;
		const int32 GetX() const;
		const int32 GetY() const;
		void Set(
			const int32 in_x,
			const int32 in_y
		);
		const bool operator==(const VectorInt2& in_rhs) const;
		const bool operator!=(const VectorInt2& in_rhs) const;
		VectorInt2& operator=(const VectorInt2& in_rhs);

		// Move to DscMath?
		//const bool Inside(
		//	const VectorFloat2& in_value
		//	) const;

	private:
		enum Index
		{
			X = 0,
			Y,
			Count
		};

	private:
		int32 _data[Index::Count];
	};
} //namespace DscCommon
