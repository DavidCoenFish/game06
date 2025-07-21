#pragma once
#include "dsc_dag.h"

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;

#if defined(_DEBUG)
	void DebugSetNodeName(NodeToken in_input, const std::string& in_debug_name);

	void DebugPrintRecurseInputs(NodeToken in_input);
	void DebugPrintRecurseOutputs(NodeToken in_input);
#endif //#if defined(_DEBUG)

} //DscDag
