#include "dsc_ui.h"
#include <dsc_common\vector_int2.h>
#include <dsc_common\vector_float2.h>

const bool operator==(const DscUi::TGradientFillConstantBuffer& in_lhs, const DscUi::TGradientFillConstantBuffer& in_rhs)
{
	for (int32 index = 0; index < 4; ++index)
	{
		if (in_lhs._colour_0[index] != in_rhs._colour_0[index])
		{
			return false;
		}
		if (in_lhs._colour_1[index] != in_rhs._colour_1[index])
		{
			return false;
		}
		if (in_lhs._colour_2[index] != in_rhs._colour_2[index])
		{
			return false;
		}
		if (in_lhs._colour_3[index] != in_rhs._colour_3[index])
		{
			return false;
		}
		if (in_lhs._focus[index] != in_rhs._focus[index])
		{
			return false;
		}
	}

	return true;
}

