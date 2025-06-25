#pragma once
#include "dsc_dag.h"

namespace DscDag
{
	enum class TValueChangeCondition
	{
		TOnValueChange = 0,
		TOnSet, // when this value is set, 
		TNotZero, // if the value set is not equavalent to zero, mark the output nodes dirty
		TNever
	};
}
