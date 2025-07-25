#include "accessor.h"
#include "dag_node_node.h"
#include "dag_node_node_array.h"

// do we just not allow DagNodeValue<std::vector<DscDag::NodeToken>> and force usage DagNodeNodeArray?
const std::vector<DscDag::NodeToken>& DscDag::GetValueNodeArray(NodeToken in_input)
{
	DSC_ASSERT(nullptr != in_input, "invalid param");
	//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "GetValueType in_input:%s in_value:%s\n", in_input->GetTypeInfo().name(), typeid(IN_TYPE).name());
	DSC_ASSERT(typeid(std::vector<DscDag::NodeToken>) == in_input->DebugGetTypeInfo(), "invalid param");

	auto value_node = dynamic_cast<DagNodeNodeArray*>(in_input);
	if (nullptr != value_node)
	{
		return value_node->GetValue();
	}

	DSC_ASSERT_ALWAYS("invalid code path");
	static std::vector<DscDag::NodeToken> kData = {};
	return kData;
}

void DscDag::NodeArrayPushBack(NodeToken in_node_array, NodeToken in_node_to_add)
{
	DSC_ASSERT(nullptr != in_node_array, "invalid param");
	DSC_ASSERT(typeid(std::vector<DscDag::NodeToken>) == in_node_array->DebugGetTypeInfo(), "invalid param");

	auto value_node = dynamic_cast<DagNodeNodeArray*>(in_node_array);
	if (nullptr != value_node)
	{
		value_node->PushBack(in_node_to_add);
	}

	return;
}

void DscDag::NodeArrayRemove(NodeToken in_node_array, NodeToken in_node_to_remove)
{
	DSC_ASSERT(nullptr != in_node_array, "invalid param");
	DSC_ASSERT(typeid(std::vector<DscDag::NodeToken>) == in_node_array->DebugGetTypeInfo(), "invalid param");

	auto value_node = dynamic_cast<DagNodeNodeArray*>(in_node_array);
	if (nullptr != value_node)
	{
		value_node->Remove(in_node_to_remove);
	}

	return;
}


// do we just not allow DagNodeValue<DscDag::NodeToken> and force usage DagNodeNode?
DscDag::NodeToken DscDag::GetValueNode(NodeToken in_input)
{
	DSC_ASSERT(nullptr != in_input, "invalid param");
	//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "GetValueType in_input:%s in_value:%s\n", in_input->GetTypeInfo().name(), typeid(IN_TYPE).name());
	DSC_ASSERT(typeid(DscDag::NodeToken) == in_input->DebugGetTypeInfo(), "invalid param");

	auto value_node = dynamic_cast<DagNodeNode*>(in_input);
	if (nullptr != value_node)
	{
		return value_node->GetValue();
	}

	DSC_ASSERT_ALWAYS("invalid code path");
	return nullptr;
}
