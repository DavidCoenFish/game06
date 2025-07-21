#include "i_dag_node.h"


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