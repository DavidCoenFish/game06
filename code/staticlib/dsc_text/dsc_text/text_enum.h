#pragma once
#include "dsc_text.h"

namespace DscText
{
	// for position of entire textblock in container
	// flag to wrap on width lives elsewhere?
	enum class THorizontalAlignment
	{
		TNone, // just follow input string, default horizontal align from locale?
		TLeft,
		TMiddle,
		TRight,
	};

	// for position of entire textblock in container
	// fixed em line height lives in text run and not in Vertical Alignment?
	enum class TVerticalAlignment
	{
		TTop,
		TMiddle,
		TBottom
	};

}
