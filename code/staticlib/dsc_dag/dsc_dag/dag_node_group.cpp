#include "dag_node_group.h"
#include "dag_collection.h"

// provide a callback for accepted enum types to construct index from
DscDag::DagNodeGroup::DagNodeGroup(const int32 in_size, const TValidateFunction& in_validate_function)
	: _validate_function(in_validate_function)
{
	DSC_ASSERT(0 < in_size, "invalid param");
	DSC_ASSERT(nullptr != in_validate_function, "invalid param");
	_node_token_array.resize(in_size);
	return;
}

DscDag::DagNodeGroup::~DagNodeGroup()
{
	DSC_ASSERT(0 == _node_ownership_group.size(), "invalid state, destroyed owned nodes before dtor, and dnont call virtual in dtor");
}

const bool DscDag::DagNodeGroup::ValidateIndexEnum(const std::type_info& in_index_enum)
{
	DSC_ASSERT(nullptr != _validate_function, "invalid param");
	if (false == _validate_function(in_index_enum))
	{
		DSC_ASSERT_ALWAYS("index enum fails supported enum");
		return false;
	}
	return true;
}

DscDag::NodeToken DscDag::DagNodeGroup::GetNodeToken(const int32 in_index) const
{
	DSC_ASSERT(0 <= in_index, "invalid param");
	DSC_ASSERT(in_index < static_cast<int32>(_node_token_array.size()), "invalid param");
	return _node_token_array[in_index];
}

void DscDag::DagNodeGroup::SetNodeToken(const int32 in_index, NodeToken in_node_token_or_null)
{
	DSC_ASSERT(0 <= in_index, "invalid param");
	DSC_ASSERT(in_index < static_cast<int32>(_node_token_array.size()), "invalid param");

	NodeToken old_value = _node_token_array[in_index];
	if (in_node_token_or_null == old_value)
	{
		return;
	}
	if (nullptr != old_value)
	{
		old_value->RemoveOutput(this);
	}
	if (nullptr != in_node_token_or_null)
	{
		in_node_token_or_null->AddOutput(this);
	}

	_node_token_array[in_index] = in_node_token_or_null;

	return;
}

void DscDag::DagNodeGroup::AddOwnership(NodeToken in_node)
{
	DSC_ASSERT(nullptr != in_node, "invalid param");
	_node_ownership_group.insert(in_node);
	return;
}

void DscDag::DagNodeGroup::DestroyOwned(DagCollection& in_dag_collection)
{
	// this was causing Unlink to be called on a destroyed node
	//for (const auto& item : _node_ownership_group)
	//{
	//	item->UnlinkInputs();
	//}

	for (const auto& item : _node_ownership_group)
	{
		in_dag_collection.DeleteNode(item);
	}

	_node_ownership_group.clear();
}

void DscDag::DagNodeGroup::MarkDirty()
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

void DscDag::DagNodeGroup::Update()
{
	if (true == _dirty)
	{
		_dirty = false;

		for (auto& item : _node_token_array)
		{
			if (nullptr != item)
			{
				item->Update();
			}
		}
	}
	return;
}

void DscDag::DagNodeGroup::AddOutput(NodeToken in_node)
{
	DSC_ASSERT(nullptr != in_node, "invalid param");
	if (nullptr != in_node)
	{
		in_node->MarkDirty();
	}
	_output.insert(in_node);
	return;
}

void DscDag::DagNodeGroup::RemoveOutput(NodeToken in_node)
{
	DSC_ASSERT(nullptr != in_node, "invalid param");
	if (nullptr != in_node)
	{
		in_node->MarkDirty();
	}
	_output.erase(in_node);
	return;
}

const bool DscDag::DagNodeGroup::GetHasNoLinks() const
{
	return (0 == _output.size());

	//for (auto& item : _node_token_array)
	//{
	//	if (nullptr != item)
	//	{
	//		return false;
	//	}
	//}
	//return true;
}

void DscDag::DagNodeGroup::UnlinkInputs()
{
	for (auto& item : _node_token_array)
	{
		if (nullptr != item)
		{
			item->RemoveOutput(this);
			//item = nullptr;
		}
	}

	for (auto& item : _node_ownership_group)
	{
		if (nullptr != item)
		{
			item->UnlinkInputs();
		}
	}

	return;
}

#if defined(_DEBUG)
const std::type_info& DscDag::DagNodeGroup::DebugGetTypeInfo() const
{
	return typeid(DscDag::NodeToken);
}

const std::string DscDag::DagNodeGroup::DebugPrintRecurseInputs(const int32 in_depth) const
{
	std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

	result += "NodeGroup:\"";
	result += DebugGetNodeName();
	result += "\" size:" + std::to_string(_node_token_array.size());
	result += " dirty:" + std::to_string(_dirty);
	result += "\n";

	result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
	result += "array:\n";
	for (NodeToken item : _node_token_array)
	{
		if (nullptr != item)
		{
			result += item->DebugPrintRecurseInputs(in_depth + 2);
		}
	}

	return result;
}

const std::string DscDag::DagNodeGroup::DebugPrintRecurseOutputs(const int32 in_depth) const
{
	std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

	result += "NodeGroup:\"";
	result += DebugGetNodeName();
	result += "\" size:" + std::to_string(_node_token_array.size());
	result += " dirty:" + std::to_string(_dirty);
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
#endif //_DEBUG