#include <dsc_dag/dag_node_value.h>

DscDag::DagNodeValue::DagNodeValue(std::any& in_value)
	: _value(in_value)
{
	// Nop
}

void DscDag::DagNodeValue::AddOutput(NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	if (nullptr != in_nodeID)
	{
		in_nodeID->MarkDirty();
	}
	_output.insert(in_nodeID);
}

void DscDag::DagNodeValue::RemoveOutput(NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	if (nullptr != in_nodeID)
	{
		in_nodeID->MarkDirty();
	}
	_output.erase(in_nodeID);
}

void DscDag::DagNodeValue::SetValue(const std::any& in_value)
{
#if 0 // provide a callback cmp operator?
	if (_value == in_value)
	{
		return;
	}
#endif
	_value = in_value;
	for (auto& item : _output)
	{
		item->MarkDirty();
	}
}

const std::any& DscDag::DagNodeValue::GetValue()
{
	return _value;
}



