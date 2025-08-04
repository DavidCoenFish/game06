#include "dag_collection.h"
#include "dag_node_node.h"
#include "dag_node_node_array.h"
#include "dag_node_group.h"

namespace
{
}

//code from google
bool DscDag::DagCollection::RawPtrComparator::operator()(const std::unique_ptr<DscDag::IDagNode>& a, const std::unique_ptr<DscDag::IDagNode>& b) const {
	return a.get() < b.get();
}

DscDag::NodeToken DscDag::DagCollection::CreateNode(
	NodeToken in_node_or_null,
	IDagOwner* const in_dag_owner_or_nullptr
)
{
	auto node = std::make_unique<DagNodeNode>(in_node_or_null);
	NodeToken node_token = node.get();
	_nodes.insert(std::move(node));
	if (nullptr != in_dag_owner_or_nullptr)
	{
		in_dag_owner_or_nullptr->AddOwnership(node_token);
	}
	return node_token;
}

DscDag::NodeToken DscDag::DagCollection::CreateNodeArray(
	const std::vector<NodeToken>& in_node_token_array,
	IDagOwner* const in_dag_owner_or_nullptr
	)
{
	auto node = std::make_unique<DagNodeNodeArray>(in_node_token_array);
	NodeToken node_token = node.get();
	_nodes.insert(std::move(node));
	if (nullptr != in_dag_owner_or_nullptr)
	{
		in_dag_owner_or_nullptr->AddOwnership(node_token);
	}
	return node_token;
}

DscDag::NodeToken DscDag::DagCollection::CreateGroup(
	const int32 in_size, 
	const TValidateFunction& in_validate_function,
	IDagOwner* const in_dag_owner_or_nullptr,
	const bool in_ignore_child_dirty
	)
{
	auto node = std::make_unique<DagNodeGroup>(in_size, in_validate_function, in_ignore_child_dirty);
	NodeToken node_token = node.get();
	_nodes.insert(std::move(node));
	if (nullptr != in_dag_owner_or_nullptr)
	{
		in_dag_owner_or_nullptr->AddOwnership(node_token);
	}
	return node_token;
}

// should already have all links removed? assert if links still exisit?
void DscDag::DagCollection::DeleteNode(NodeToken in_node)
{
	DSC_ASSERT(nullptr != in_node, "invalid param");
	//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "DeleteNode:%s\n", in_node->DebugGetNodeName().c_str());

	// if this is a node group, it adds output to each internal node, unlink them BEFORE we get up to the destroy owned
	// as node groups can both have linkage via SetNode and ownership
	// yes this is a mess, but lets see how far we can go
	in_node->UnlinkInputs();

	IDagOwner* dag_owner = dynamic_cast<IDagOwner*>(in_node);
	if (nullptr != dag_owner)
	{
		dag_owner->DestroyOwned(*this);
	}
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



