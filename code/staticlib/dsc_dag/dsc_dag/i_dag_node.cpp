#include <dsc_dag/i_dag_node.h>

DscDag::IDagNode::IDagNode(DSC_DEBUG_ONLY(const std::string& in_debug_name))
DSC_DEBUG_ONLY(: _debug_name(in_debug_name))
{
	//nop
}

DscDag::IDagNode::~IDagNode() {}

void DscDag::IDagNode::MarkDirty()
{
	DSC_ASSERT_ALWAYS("invalid code path");
}

void DscDag::IDagNode::Update()
{
	// harmless to call on a DagValue?
	//nop
}

void DscDag::IDagNode::AddOutput(NodeToken)
{
	DSC_ASSERT_ALWAYS("invalid code path");
}

void DscDag::IDagNode::RemoveOutput(NodeToken)
{
	DSC_ASSERT_ALWAYS("invalid code path");
}

void DscDag::IDagNode::SetIndexInput(const int32, NodeToken)
{
	DSC_ASSERT_ALWAYS("invalid code path");
}

void DscDag::IDagNode::AddInput(NodeToken)
{
	DSC_ASSERT_ALWAYS("invalid code path");
}

void DscDag::IDagNode::RemoveInput(NodeToken)
{
	DSC_ASSERT_ALWAYS("invalid code path");
}

void DscDag::IDagNode::SetFromNode(IDagNode* const)
{
	DSC_ASSERT_ALWAYS("invalid code path");
}
