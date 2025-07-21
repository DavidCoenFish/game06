#include "link.h"
#include "i_dag_node.h"
#include "dag_node_calculate.h"

void DscDag::LinkNodes(NodeToken in_input, NodeToken in_output)
{
	DSC_ASSERT(nullptr != in_input, "invalid param");
	DSC_ASSERT(nullptr != in_output, "invalid param");
	if ((nullptr != in_input) && (nullptr != in_output))
	{
		in_input->AddOutput(in_output);

		auto calculate_node = dynamic_cast<DagNodeCalculateBase*>(in_output);
		DSC_ASSERT(nullptr != calculate_node, "invalid state");
		if (nullptr != calculate_node)
		{
			calculate_node->AddInput(in_input);
		}
	}
	return;
}

void DscDag::LinkIndexNodes(int32 in_index, NodeToken in_input, NodeToken in_output)
{
	bool outputs_input_changed = false;
	if (nullptr != in_output)
	{
		auto calculate_node = dynamic_cast<DagNodeCalculateBase*>(in_output);
		DSC_ASSERT(nullptr != calculate_node, "invalid state");
		if (nullptr != calculate_node)
		{
			outputs_input_changed = calculate_node->SetIndexInput(in_index, in_input);
		}
	}

	// trying to allow redundant calls to LinkIndexNodes, this is not idea, as REPLACING a link could end up with a linkage leak...?
	if ((true == outputs_input_changed) && (nullptr != in_input) && (nullptr != in_output))
	{
		in_input->AddOutput(in_output);
	}

	return;
}

void DscDag::UnlinkNodes(NodeToken in_input, NodeToken in_output)
{
	DSC_ASSERT(nullptr != in_input, "invalid param");
	DSC_ASSERT(nullptr != in_output, "invalid param");
	if ((nullptr != in_input) && (nullptr != in_output))
	{
		auto calculate_node = dynamic_cast<DagNodeCalculateBase*>(in_output);
		DSC_ASSERT(nullptr != calculate_node, "invalid state");
		if (nullptr != calculate_node)
		{
			calculate_node->RemoveInput(in_input);
		}

		in_input->RemoveOutput(in_output);
	}
	return;
}

void DscDag::UnlinkIndexNodes(int32 in_index, NodeToken in_input, NodeToken in_output)
{
	DSC_ASSERT(nullptr != in_output, "invalid param");
	if (nullptr != in_input)
	{
		in_input->RemoveOutput(in_output);
	}
	if (nullptr != in_output)
	{
		auto calculate_node = dynamic_cast<DagNodeCalculateBase*>(in_output);
		DSC_ASSERT(nullptr != calculate_node, "invalid state");
		if (nullptr != calculate_node)
		{
			calculate_node->SetIndexInput(in_index, nullptr);
		}
	}
	return;
}

