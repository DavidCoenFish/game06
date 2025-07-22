#pragma once

#define LOG_TOPIC_DSC_DAG "DSC_DAG"

// allow the IDE to get definitions, otherwise the include paths are defined in the BFF script outside awarness of the IDE
// added DSC_BFF_BUILD to fastbuild defines
#ifndef DSC_BFF_BUILD

#include "..\..\dsc_common\dsc_common\dsc_common.h"

#include "accessor.h"
#include "dag_collection.h"
#include "dag_node_calculate.h"
#include "dag_node_condition.h"
#include "dag_node_group.h"
#include "dag_node_node.h"
#include "dag_node_node_array.h"
#include "dag_node_value.h"
#include "debug_print.h"
#include "i_dag_node.h"
#include "i_dag_owner.h"
#include "link.h"

#endif //#ifndef DSC_BFF_BUILD

#include <dsc_common/dsc_common.h>