#include "dag_collection.h"
#include "dag_node_calculate.h"
#include "dag_node_condition.h"
#include "dag_node_value.h"

namespace
{
}

//code from google
bool DscDag::DagCollection::RawPtrComparator::operator()(const std::unique_ptr<DscDag::IDagNode>& a, const std::unique_ptr<DscDag::IDagNode>& b) const {
	return a.get() < b.get();
}

// should already have all links removed? assert if links still exisit?
void DscDag::DagCollection::DeleteNode(NodeToken in_node)
{
	DSC_ASSERT(nullptr != in_node, "invalid param");
	DSC_ASSERT(true == in_node->GetHasNoLinks(), "invalid state");
	//_nodes.find(in_node);
	auto it = std::find_if(_nodes.begin(), _nodes.end(), [&](const std::unique_ptr<DscDag::IDagNode>& ptr) {
		return ptr.get() == in_node;
	});
	if (it != _nodes.end())
	{
		_nodes.erase(it);
	}
	else
	{
		DSC_ASSERT_ALWAYS("attempt to delete a node which was not found");
	}

	//todo: do we need to also check _named_nodes and _dirty_condition_nodes

	return;
}

void DscDag::DagCollection::AddDirtyConditionNode(NodeToken in_dirty_condition_node)
{
	_dirty_condition_nodes.insert(in_dirty_condition_node);
}

void DscDag::DagCollection::ResolveDirtyConditionNodes()
{
	while (0 < _dirty_condition_nodes.size())
	{
		std::set<NodeToken> temp = {};
		std::swap(_dirty_condition_nodes, temp);
		for (auto& item : temp)
		{
			item->Update();
		}
	}
	return;
}

void DscDag::DagCollection::AddNodeName(NodeToken in_node, const std::string& in_name)
{
	_named_nodes.insert(std::make_pair(in_name, in_node));
}

void DscDag::DagCollection::RemoveNodeName(const std::string& in_name)
{
	auto found = _named_nodes.find(in_name);
	if (found != _named_nodes.end())
	{
		_named_nodes.erase(found);
	}
	return;
}

DscDag::NodeToken DscDag::DagCollection::FetchNodeName(const std::string& in_name)
{
	auto found = _named_nodes.find(in_name);
	if (found != _named_nodes.end())
	{
		return found->second;
	}
	return nullptr;
}

const int32 DscDag::DagCollection::GetNodeCount() const
{
	return static_cast<int32>(_nodes.size());
}



