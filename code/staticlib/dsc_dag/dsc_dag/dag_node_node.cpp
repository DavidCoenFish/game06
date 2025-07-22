#include "dag_node_node.h"
#include <dsc_common\debug_print.h>

DscDag::DagNodeNode::DagNodeNode(NodeToken in_node_or_null)
	: _node(in_node_or_null)
{
	//nop
}

DscDag::NodeToken DscDag::DagNodeNode::Get() const
{
	return _node;
}

void DscDag::DagNodeNode::Set(NodeToken in_node_or_null)
{
	if (_node == in_node_or_null)
	{
		return;
	}
	if (nullptr != _node)
	{
		_node->RemoveOutput(this);
	}
	if (nullptr != in_node_or_null)
	{
		in_node_or_null->AddOutput(this);
	}

	_node = in_node_or_null;

	return;
}

void DscDag::DagNodeNode::MarkDirty()
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

	return;
}

void DscDag::DagNodeNode::Update()
{
	if (true == _dirty)
	{
		_dirty = false;
		if (nullptr != _node)
		{
			_node->Update();
		}
	}
	return;
}

void DscDag::DagNodeNode::AddOutput(NodeToken in_node)
{
	DSC_ASSERT(nullptr != in_node, "invalid param");
	if (nullptr != in_node)
	{
		in_node->MarkDirty();
	}
	_output.insert(in_node);
}

void DscDag::DagNodeNode::RemoveOutput(NodeToken in_node)
{
	DSC_ASSERT(nullptr != in_node, "invalid param");
	if (nullptr != in_node)
	{
		in_node->MarkDirty();
	}
	_output.erase(in_node);
}

const bool DscDag::DagNodeNode::GetHasNoLinks() const
{
	if (0 != _output.size())
	{
		return false;
	}
	if (nullptr != _node)
	{
		return false;
	}
	return true;
}

void DscDag::DagNodeNode::UnlinkInputs()
{
	if (nullptr != _node)
	{
		_node->RemoveOutput(this);
		_node = nullptr;
	}

	return;
}

#if defined(_DEBUG)
const std::type_info& DscDag::DagNodeNode::DebugGetTypeInfo() const
{
	return typeid(NodeToken);
}

const std::string DscDag::DagNodeNode::DebugPrintRecurseInputs(const int32 in_depth) const
{
	std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

	result += "Node:\"";
	result += DebugGetNodeName();
	result += "\" dirty:" + std::to_string(_dirty);
	result += "\n";

	if (nullptr != _node)
	{
		result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
		result += "input:\n";
		result += _node->DebugPrintRecurseInputs(in_depth + 2);
	}

	return result;
}

const std::string DscDag::DagNodeNode::DebugPrintRecurseOutputs(const int32 in_depth) const
{
	std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

	result += "Node:\"";
	result += DebugGetNodeName();
	result += "\" dirty:" + std::to_string(_dirty);
	result += "\n";

	if (0 < _output.size())
	{
		result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
		result += "output:\n";
		for (NodeToken item : _output)
		{
			if (nullptr != item)
			{
				result += item->DebugPrintRecurseOutputs(in_depth + 2);
			}
		}
	}

	return result;
}
#endif //#if defined(_DEBUG)

