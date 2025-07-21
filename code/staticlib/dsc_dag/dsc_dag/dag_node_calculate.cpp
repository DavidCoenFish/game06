#include "dag_node_calculate.h"

const bool DscDag::DagNodeCalculateBase::SetIndexInput(const int32 in_index, NodeToken in_node)
{
	DSC_ASSERT(0 <= in_index, "invalid param");
	// null in_nodeID allowed
	if (static_cast<int32>(_index_input.size()) <= in_index)
	{
		_index_input.resize(in_index + 1);
	}
	// only marking dirty if the link state changes
	bool result = false;
	if (_index_input[in_index] != in_node)
	{
		result = true;
		_index_input[in_index] = in_node;
		MarkDirty();
	}
	return result;
}

void DscDag::DagNodeCalculateBase::AddInput(NodeToken in_node)
{
	DSC_ASSERT(nullptr != in_node, "invalid param");
	_input.insert(in_node);
	MarkDirty();
}

void DscDag::DagNodeCalculateBase::RemoveInput(NodeToken in_node)
{
	DSC_ASSERT(nullptr != in_node, "invalid param");
	_input.erase(in_node);
	MarkDirty();
}
