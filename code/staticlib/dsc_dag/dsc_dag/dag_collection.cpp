#include "dag_collection.h"
#include "dag_node_value.h"
#include "dag_node_calculate.h"

namespace
{
}

//code from google
bool DscDag::DagCollection::RawPtrComparator::operator()(const std::unique_ptr<DscDag::IDagNode>& a, const std::unique_ptr<DscDag::IDagNode>& b) const {
	return a.get() < b.get();
}

DscDag::NodeToken DscDag::DagCollection::CreateValue(const std::any& in_value, const TValueChangeCondition in_change_condition)
{
	auto node = std::make_unique<DagNodeValue>(in_value, in_change_condition);
	NodeToken nodeToken = node.get();
	_nodes.insert(std::move(node));
	return nodeToken;
}

DscDag::NodeToken DscDag::DagCollection::CreateCalculate(const TCalculateFunction& in_calculate)
{
	auto node = std::make_unique<DagNodeCalculate>(in_calculate);
	NodeToken nodeToken = node.get();
	_nodes.insert(std::move(node));
	return nodeToken;
}

DscDag::NodeToken DscDag::DagCollection::AddCustomNode(std::unique_ptr<IDagNode>&& in_node)
{
	NodeToken nodeToken = in_node.get();
	_nodes.insert(std::move(in_node));
	return nodeToken;
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

	return;
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

