#pragma once

#define LOG_TOPIC_DSC_DAG_2 "DSC_DAG_2"

namespace DscDag2
{
	enum class EMarkValueDirtyLogic
	{
		TNone,
		TValueChange,
		TNotZero // if you set a value and it is not zero, make dependents dirty
	};
}

// allow the IDE to get definitions, otherwise the include paths are defined in the BFF script outside awarness of the IDE
// added DSC_BFF_BUILD to fastbuild defines
#ifndef DSC_BFF_BUILD

#include "..\..\dsc_common\dsc_common\dsc_common.h"

#include "dag_2_calculate_component.h"
#include "dag_2_dirty_component.h"
#include "dag_2_node.h"
#include "i_dag_2_calculate_component.h"


#endif //#ifndef DSC_BFF_BUILD

#include <dsc_common/dsc_common.h>