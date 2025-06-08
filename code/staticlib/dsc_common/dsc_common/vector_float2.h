#pragma once
#include "dsc_common.h"

namespace DscCommon
{
	class VectorFloat2
	{
	public:
		static const VectorFloat2 s_zero;

		explicit VectorFloat2(
			const float in_x = 0,
			const float in_y = 0
		);
		float& operator[](const int in_index);
		const float operator[](const int in_index) const;
		const float GetX() const;
		const float GetY() const;
		void Set(
			const float in_x,
			const float in_y
		);
		const bool operator==(const VectorFloat2& in_rhs) const;
		const bool operator!=(const VectorFloat2& in_rhs) const;
		VectorFloat2& operator=(const VectorFloat2& in_rhs);

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
		float _data[Index::Count];
	};
} //namespace DscCommon
