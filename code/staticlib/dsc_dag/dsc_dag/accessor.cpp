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
