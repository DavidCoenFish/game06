#include "i_dag_node.h"
#include <dsc_common\log_system.h>

void DscDag::IDagNode::MarkDirty()
{
	DSC_ASSERT_ALWAYS("not actually expecting this to be called");
}

void DscDag::IDagNode::Update()
{
	//DSC_ASSERT_ALWAYS("not actually expecting this to be called, or a value could be held by a DagNodeGroup and have update called");
	//nop
}

const bool DscDag::IDagNode::GetHasNoLinks() const
{
	return true;
}

void DscDag::IDagNode::UnlinkInputs()
{
	//nop
}
#if defined(_DEBUG)
void DscDag::IDagNode::DebugSetNodeName(const std::string& in_debug_name)
{
	//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "node:%p %s\n", this, in_debug_name.c_str());
	_debug_name = in_debug_name;
}
#endif//#if defined(_DEBUG)
