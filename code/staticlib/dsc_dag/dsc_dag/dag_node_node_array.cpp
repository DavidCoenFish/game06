#include "dag_node_node_array.h"
#include <dsc_common\debug_print.h>

DscDag::DagNodeNodeArray::DagNodeNodeArray(const std::vector<NodeToken>& in_node_array)
{
	_node_array.reserve(in_node_array.size());
	for (auto& item : in_node_array)
	{
		PushBack(item);
	}
}

void DscDag::DagNodeNodeArray::PushBack(NodeToken in_node)
{
	DSC_ASSERT(nullptr != in_node, "invalid param");
	if (nullptr != in_node)
	{
		in_node->AddOutput(this);
		_node_array.push_back(in_node);
		MarkDirty();
	}
	return;
}

void DscDag::DagNodeNodeArray::Remove(NodeToken in_node)
{
	DSC_ASSERT(nullptr != in_node, "invalid param");
	if (nullptr != in_node)
	{
		in_node->RemoveOutput(this);
		//https://stackoverflow.com/questions/39912/how-do-i-remove-an-item-from-a-stl-vector-with-a-certain-value
		_node_array.erase(std::remove(_node_array.begin(), _node_array.end(), in_node), _node_array.end());
		MarkDirty();
	}
	return;
}

DscDag::NodeToken DscDag::DagNodeNodeArray::GetAt(const int32 in_index) const
{
	DSC_ASSERT(0 <= in_index, "invalid param");
	if ((0 <= in_index) && (in_index < static_cast<int32>(_node_array.size())))
	{
		return _node_array[in_index];
	}
	return nullptr;
}

void DscDag::DagNodeNodeArray::SetAt(NodeToken in_node_or_null, const int32 in_index)
{
	DSC_ASSERT(0 <= in_index, "invalid param");

	if (static_cast<int32>(_node_array.size()) <= in_index)
	{
		_node_array.resize(in_index + 1);
	}

	NodeToken old_node = _node_array[in_index];
	if (old_node == in_node_or_null)
	{
		return;
	}
	if (nullptr != old_node)
	{
		old_node->RemoveOutput(this);
	}
	if (nullptr != in_node_or_null)
	{
		in_node_or_null->AddOutput(this);
	}
	_node_array[in_index] = in_node_or_null;

	MarkDirty();

	return;
}

const int32 DscDag::DagNodeNodeArray::GetCount() const
{
	return static_cast<int32>(_node_array.size());
}

void DscDag::DagNodeNodeArray::MarkDirty()
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

void DscDag::DagNodeNodeArray::Update()
{
	if (true == _dirty)
	{
		_dirty = false;
		for (auto& item : _node_array)
		{
			if (nullptr != item)
			{
				item->Update();
			}
		}
	}
	return;
}

void DscDag::DagNodeNodeArray::AddOutput(NodeToken in_node)
{
	DSC_ASSERT(nullptr != in_node, "invalid param");
	if (nullptr != in_node)
	{
		in_node->MarkDirty();
	}
	_output.insert(in_node);
}

void DscDag::DagNodeNodeArray::RemoveOutput(NodeToken in_node)
{
	DSC_ASSERT(nullptr != in_node, "invalid param");
	if (nullptr != in_node)
	{
		in_node->MarkDirty();
	}
	_output.erase(in_node);
}

const bool DscDag::DagNodeNodeArray::GetHasNoLinks() const
{
	if (0 != _output.size())
	{
		return false;
	}
	//for (auto& item : _node_array)
	//{
	//	if (nullptr != item)
	//	{
	//		return false;
	//	}
	//}
	return true;
}

void DscDag::DagNodeNodeArray::UnlinkInputs()
{
	if (false == _unlinked)
	{
		_unlinked = true;
		for (auto& item : _node_array)
		{
			if (nullptr != item)
			{
				//item->UnlinkInputs();
				item->RemoveOutput(this);
			}
		}
		//_node_array.clear();
	}

	return;
}

#if defined(_DEBUG)
const std::type_info& DscDag::DagNodeNodeArray::DebugGetTypeInfo() const
{
	return typeid(std::vector<NodeToken>);
}

const std::string DscDag::DagNodeNodeArray::DebugPrintRecurseInputs(const int32 in_depth) const
{
	std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

	result += "Node array:\"";
	result += DebugGetNodeName();
	result += "\" dirty:" + std::to_string(_dirty);
	result += "\n";

	for (auto& item : _node_array)
	{
		if (nullptr != item)
		{
			result += item->DebugPrintRecurseInputs(in_depth + 1);
		}
	}

	return result;
}

const std::string DscDag::DagNodeNodeArray::DebugPrintRecurseOutputs(const int32 in_depth) const
{
	std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

	result += "Node array:\"";
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

