#pragma once
#include "dsc_dag.h"
#include "dag_node_array.h"
#include "dag_node_calculate.h"
#include "dag_node_value.h"

namespace DscDag
{
	template <typename IN_TYPE>
	static const IN_TYPE& GetValueType(NodeToken in_input)
	{
		DSC_ASSERT(nullptr != in_input, "invalid param");
		//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "GetValueType in_input:%s in_value:%s\n", in_input->GetTypeInfo().name(), typeid(IN_TYPE).name());
		DSC_ASSERT(typeid(IN_TYPE) == in_input->GetTypeInfo(), "invalid param");

		auto value_node = dynamic_cast<DagNodeValue< IN_TYPE>*>(in_input);
		if (nullptr != value_node)
		{
			return value_node->GetValue();
		}

		auto calculate_node = dynamic_cast<DagNodeCalculate< IN_TYPE>*>(in_input);
		if (nullptr != calculate_node)
		{
			return calculate_node->GetValue();
		}

		DSC_ASSERT_ALWAYS("invalid code path");
		static IN_TYPE kData = {};
		return kData;
	}

	template <typename IN_TYPE>
	static IN_TYPE& GetValueNonConstRef(NodeToken in_input, const bool in_set_dirty)
	{
		DSC_ASSERT(nullptr != in_input, "invalid param");
		//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "GetValueType in_input:%s in_value:%s\n", in_input->GetTypeInfo().name(), typeid(IN_TYPE).name());
		DSC_ASSERT(typeid(IN_TYPE) == in_input->GetTypeInfo(), "invalid param");

		auto value_node = dynamic_cast<DagNodeValue< IN_TYPE>*>(in_input);
		if (nullptr != value_node)
		{
			return value_node->GetValueNonConst(in_set_dirty);
		}

		DSC_ASSERT_ALWAYS("invalid code path");
		static IN_TYPE kData = {};
		return kData;
	}

	template <typename IN_TYPE>
	static void SetValueType(NodeToken in_input, const IN_TYPE& in_value)
	{
		DSC_ASSERT(nullptr != in_input, "invalid param");
		//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "SetValueType in_input:%s in_value:%s\n", in_input->GetTypeInfo().name(), typeid(IN_TYPE).name());
		DSC_ASSERT(typeid(IN_TYPE) == in_input->GetTypeInfo(), "invalid param");

		auto value_node = dynamic_cast<DagNodeValue< IN_TYPE>*>(in_input);
		if (nullptr != value_node)
		{
			value_node->SetValue(in_value);
		}
		else
		{
			DSC_ASSERT_ALWAYS("invalid code path");
		}
	}


} //DscDag
