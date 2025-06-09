#pragma once
#include "dsc_text.h"

namespace DscText
{
	enum class THorizontalAlignment
	{
		TNone, // just follow input string, default horizontal align from locale?
		TForceLeft,
		TForceMiddle,
		TForceRight,
	};
}
