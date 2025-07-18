#pragma once
#include <dsc_common/dsc_common.h>

#define LOG_TOPIC_DSC_DAG_PRINT "DSC_DAG_PRINT"

// allow the IDE to get definitions, otherwise the include paths are defined in the BFF script outside awarness of the IDE
// added DSC_BFF_BUILD to fastbuild defines
#ifndef DSC_BFF_BUILD

//G:\development\game06\code\staticlib\dsc_dag\dsc_dag\dsc_dag.h
#include "..\..\dsc_common\dsc_common\dsc_common.h"
#include "..\..\dsc_dag\dsc_dag\dsc_dag.h"

#endif //#ifndef DSC_BFF_BUILD

namespace DscDagPrint
{
#if defined(_DEBUG)
	void SetDebugPrintMethods();
#endif //#if defined(_DEBUG)
}
