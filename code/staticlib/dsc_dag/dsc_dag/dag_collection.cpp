#include <dsc_dag/dag_collection.h>
#include <dsc_dag/dag_node_value.h>
#include <dsc_dag/dag_node_calculate.h>

DscDag::NodeToken DscDag::DagCollection::CreateValue(const std::any& in_value)
{
	auto node = std::make_unique<DagNodeValue>(in_value);
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

