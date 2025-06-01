#pragma once

#include <dsc_common\dsc_common.h>

namespace DscCommon
{
	class VectorFloat4
	{
	public:
		static const VectorFloat4 s_zero;
		static const VectorFloat4 s_white;
		static const VectorFloat4 s_black;
		static const VectorFloat4 s_render_coordinates_full;

		explicit VectorFloat4(
			const float in_x = 0,
			const float in_y = 0,
			const float in_z = 0,
			const float in_w = 0
		);
		float& operator[](const int in_index);
		const float operator[](const int in_index) const;
		const float GetX() const;
		const float GetY() const;
		const float GetZ() const;
		const float GetW() const;
		void Set(
			const float in_x,
			const float in_y,
			const float in_z,
			const float in_w
		);
		const bool operator==(const VectorFloat4& in_rhs) const;
		const bool operator!=(const VectorFloat4& in_rhs) const;
		VectorFloat4& operator=(const VectorFloat4& in_rhs);

		// Move to DscMath?
		//const bool Inside(
		//	const VectorFloat2& in_value
		//	) const;

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
		float _data[Index::Count];
	};
} //namespace DscCommon
