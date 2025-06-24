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

DscDag::NodeToken DscDag::DagCollection::CreateValue(const std::any& in_value, const TValueChangeCondition in_change_condition  DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name))
{
	auto node = std::make_unique<DagNodeValue>(in_value, in_change_condition DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
	NodeToken node_token = node.get();
	_nodes.insert(std::move(node));
	return node_token;
}

DscDag::NodeToken DscDag::DagCollection::CreateCalculate(const TCalculateFunction& in_calculate  DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name))
{
	auto node = std::make_unique<DagNodeCalculate>(in_calculate DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
	NodeToken node_token = node.get();
	_nodes.insert(std::move(node));
	return node_token;
}

DscDag::NodeToken DscDag::DagCollection::CreateCondition(NodeToken in_condition, NodeToken in_true_source, NodeToken in_false_source, NodeToken in_true_destination, NodeToken in_false_destination DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name))
{
	auto node = std::make_unique<DagNodeCondition>(*this, in_true_destination, in_false_destination DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
	NodeToken node_token = node.get();
	_nodes.insert(std::move(node));

	if (nullptr != in_condition)
	{
		LinkIndexNodes(0, in_condition, node_token);
	}
	if (nullptr != in_true_source)
	{
		LinkIndexNodes(1, in_true_source, node_token);
	}
	if (nullptr != in_false_source)
	{
		LinkIndexNodes(2, in_false_source, node_token);
	}

	return node_token;

}

DscDag::NodeToken DscDag::DagCollection::AddCustomNode(std::unique_ptr<IDagNode>&& in_node)
{
	NodeToken node_token = in_node.get();
	_nodes.insert(std::move(in_node));
	return node_token;
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
			item->GetValue();
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

void DscDag::DagCollection::LinkNodes(NodeToken in_input, NodeToken in_output)
{
	DSC_ASSERT(nullptr != in_input, "invalid param");
	DSC_ASSERT(nullptr != in_output, "invalid param");
	if ((nullptr != in_input) && (nullptr != in_output))
	{
		in_output->AddInput(in_input);
		in_input->AddOutput(in_output);
	}
	return;
}

void DscDag::DagCollection::LinkIndexNodes(int32 in_index, NodeToken in_input, NodeToken in_output)
{
	DSC_ASSERT(nullptr != in_input, "invalid param");
	DSC_ASSERT(nullptr != in_output, "invalid param");
	if ((nullptr != in_input) && (nullptr != in_output))
	{
		in_output->SetIndexInput(in_index, in_input);
		in_input->AddOutput(in_output);
	}
	return;
}

void DscDag::DagCollection::UnlinkNodes(NodeToken in_input, NodeToken in_output)
{
	DSC_ASSERT(nullptr != in_input, "invalid param");
	DSC_ASSERT(nullptr != in_output, "invalid param");
	if ((nullptr != in_input) && (nullptr != in_output))
	{
		in_output->RemoveInput(in_input);
		in_input->RemoveOutput(in_output);
	}
	return;
}

void DscDag::DagCollection::UnlinkIndexNodes(int32 in_index, NodeToken in_input, NodeToken in_output)
{
	DSC_ASSERT(nullptr != in_output, "invalid param");
	if (nullptr != in_input) 
	{
		in_input->RemoveOutput(in_output);
	}
	if (nullptr != in_output)
	{
		in_output->SetIndexInput(in_index, nullptr);
	}
	return;
}

const std::any& DscDag::DagCollection::GetValue(NodeToken in_input)
{
	DSC_ASSERT(nullptr != in_input, "invalid param");
	return in_input->GetValue();
}

void DscDag::DagCollection::SetValue(NodeToken in_input, const std::any& in_value)
{
	DSC_ASSERT(nullptr != in_input, "invalid param");
	in_input->SetValue(in_value);
	return;
}

