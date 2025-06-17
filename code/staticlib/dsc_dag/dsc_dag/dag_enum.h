#pragma once
#include "dsc_dag.h"

namespace DscDag
{
	enum class TValueChangeCondition
	{
		TOnValueChange = 0,
		TOnSet,
		TNever
	};
}
