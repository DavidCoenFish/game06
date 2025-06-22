#include "dag_node_calculate.h"

DscDag::DagNodeCalculate::DagNodeCalculate(const TCalculateFunction& in_calculateFunction DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name))
	: IDagNode(DSC_DEBUG_ONLY(in_debug_name))
	, _calculateFunction(in_calculateFunction)
{
	// nop
}

void DscDag::DagNodeCalculate::MarkDirty()
{
	if (true == _dirty)
	{
		return;
	}
	_dirty = true;
	for (auto& item : _output)
	{
		item->MarkDirty();
	}
}

void DscDag::DagNodeCalculate::AddOutput(NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	if (nullptr != in_nodeID)
	{
		in_nodeID->MarkDirty();
	}
	_output.insert(in_nodeID);
}

void DscDag::DagNodeCalculate::RemoveOutput(NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	if (nullptr != in_nodeID)
	{
		in_nodeID->MarkDirty();
	}
	_output.erase(in_nodeID);
}

void DscDag::DagNodeCalculate::SetIndexInput(const int32 in_index, NodeToken in_nodeID)
{
	DSC_ASSERT(0 <= in_index, "invalid param");
	// null in_nodeID allowed
	if (static_cast<int32>(_indexInput.size()) <= in_index)
	{
		_indexInput.resize(in_index + 1);
	}
	_indexInput[in_index] = in_nodeID;
	MarkDirty();
}

DscDag::NodeToken DscDag::DagNodeCalculate::GetIndexInput(const int32 in_index) const
{
	DSC_ASSERT(0 <= in_index, "invalid param");
	// null in_nodeID allowed
	if ((0 <= in_index) && (in_index < static_cast<int32>(_indexInput.size())))
	{
		return _indexInput[in_index];
	}
	return nullptr;
}

void DscDag::DagNodeCalculate::AddInput(NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	_input.insert(in_nodeID);
	MarkDirty();
}

void DscDag::DagNodeCalculate::RemoveInput(NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	_input.erase(in_nodeID);
	MarkDirty();
}

const std::any& DscDag::DagNodeCalculate::GetValue()
{
	if (true == _dirty)
	{
		_dirty = false;
		_calculateFunction(_value, _input, _indexInput);
	}
	return _value;
}


const bool DscDag::DagNodeCalculate::GetHasNoLinks() const
{
	if (0 != _output.size())
	{
		return false;
	}
	if (0 != _input.size())
	{
		return false;
	}
	for (const auto& item : _indexInput)
	{
		if (nullptr != item)
		{
			return false;
		}
	}
	return true;
}

