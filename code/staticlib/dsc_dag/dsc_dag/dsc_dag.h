#pragma once

#define LOG_TOPIC_DSC_DAG "DSC_DAG"

// allow the IDE to get definitions, otherwise the include paths are defined in the BFF script outside awarness of the IDE
// added DSC_BFF_BUILD to fastbuild defines
#ifndef DSC_BFF_BUILD

//G:\development\game06\code\staticlib\dsc_dag\dsc_dag\dsc_dag.h
#include "..\..\dsc_common\dsc_common\dsc_common.h"

#include "dag_collection.h"
#include "dag_enum.h"
#include "dag_group.h"
#include "dag_node_calculate.h"
#include "dag_node_condition.h"
#include "dag_node_value.h"
#include "dag_node_value_unique.h"
#include "i_dag_node.h"

#endif //#ifndef DSC_BFF_BUILD

#include <dsc_common/dsc_common.h>