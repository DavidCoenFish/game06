#pragma once
#include "dsc_dag.h"
#include <dsc_common\dsc_common.h>
#include <dsc_common\log_system.h>
#include <dsc_dag\i_dag_node.h>
#include <dsc_dag\dag_enum.h>
#include <dsc_dag\dag_node_value.h>
#include <dsc_dag\dag_node_value_unique.h>
#include <dsc_dag\dag_node_calculate.h>

namespace DscDag
{
	class IDagNode;
	class DagNodeCondition;
	
	namespace ArrayHelper
	{
		template <typename IN_TYPE>
		static const int32 GetArraySize(NodeToken in_input)
		{
			DSC_ASSERT(nullptr != in_input, "invalid param");
			//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "GetArraySize in_input:%s IN_TYPE:%s\n", in_input->GetTypeInfo().name(), typeid(std::vector<IN_TYPE>).name());
			DSC_ASSERT(typeid(std::vector<IN_TYPE>) == in_input->GetTypeInfo(), "invalid param");

			auto value_node = dynamic_cast<DagNodeValue<std::vector<IN_TYPE>>*>(in_input);
			if (nullptr != value_node)
			{
				return value_node->GetValue().size();
			}

			auto calculate_node = dynamic_cast<DagNodeCalculate<std::vector<IN_TYPE>>*>(in_input);
			if (nullptr != calculate_node)
			{
				return calculate_node->GetValue().size();
			}

			auto value_unique_node = dynamic_cast<DagNodeValueUnique<std::vector<IN_TYPE>>*>(in_input);
			if (nullptr != value_unique_node)
			{
				return value_unique_node->GetValue().size();
			}

			DSC_ASSERT_ALWAYS("invalid code path");
			return 0;
		}

		//at
		template <typename IN_TYPE>
		static const IN_TYPE& At(NodeToken in_input, const int32 in_index)
		{
			DSC_ASSERT(nullptr != in_input, "invalid param");
			//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "GetArraySize in_input:%s IN_TYPE:%s\n", in_input->GetTypeInfo().name(), typeid(std::vector<IN_TYPE>).name());
			DSC_ASSERT(typeid(std::vector<IN_TYPE>) == in_input->GetTypeInfo(), "invalid param");

			auto value_node = dynamic_cast<DagNodeValue<std::vector<IN_TYPE>>*>(in_input);
			if (nullptr != value_node)
			{
				return value_node->GetValue()[in_index];
			}

			auto calculate_node = dynamic_cast<DagNodeCalculate<std::vector<IN_TYPE>>*>(in_input);
			if (nullptr != calculate_node)
			{
				return calculate_node->GetValue()[in_index];
			}

			auto value_unique_node = dynamic_cast<DagNodeValueUnique<std::vector<IN_TYPE>>*>(in_input);
			if (nullptr != value_unique_node)
			{
				return value_unique_node->GetValue()[in_index];
			}

			DSC_ASSERT_ALWAYS("invalid code path");
			static const IN_TYPE s_dummy = {};
			return s_dummy;
		}

		//push
		template <typename IN_TYPE>
		static void PushBack(NodeToken in_input, const IN_TYPE& in_value, const bool in_set_dirty = true)
		{
			DSC_ASSERT(nullptr != in_input, "invalid param");
			//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "GetArraySize in_input:%s IN_TYPE:%s\n", in_input->GetTypeInfo().name(), typeid(std::vector<IN_TYPE>).name());
			DSC_ASSERT(typeid(std::vector<IN_TYPE>) == in_input->GetTypeInfo(), "invalid param");

			auto value_node = dynamic_cast<DagNodeValue<std::vector<IN_TYPE>>*>(in_input);
			if (nullptr != value_node)
			{
				value_node->GetValueNonConst(in_set_dirty).push_back(in_value);
				return;
			}

			auto value_unique_node = dynamic_cast<DagNodeValueUnique<std::vector<IN_TYPE>>*>(in_input);
			if (nullptr != value_unique_node)
			{
				value_unique_node->GetValue(true)->push_back(in_value);
				return;
			}

			DSC_ASSERT_ALWAYS("invalid code path");
			return;
		}

		template <typename IN_TYPE>
		static void Remove(NodeToken in_input, const IN_TYPE& in_value, const bool in_set_dirty = true)
		{
			DSC_ASSERT(nullptr != in_input, "invalid param");
			//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "GetArraySize in_input:%s IN_TYPE:%s\n", in_input->GetTypeInfo().name(), typeid(std::vector<IN_TYPE>).name());
			DSC_ASSERT(typeid(std::vector<IN_TYPE>) == in_input->GetTypeInfo(), "invalid param");
			auto value_node = dynamic_cast<DagNodeValue<std::vector<IN_TYPE>>*>(in_input);
			if (nullptr != value_node)
			{
				auto& array = value_node->GetValueNonConst(in_set_dirty);
				array.erase(std::remove(array.begin(), array.end(), in_input), array.end());
				return;
			}

			auto value_unique_node = dynamic_cast<DagNodeValueUnique<std::vector<IN_TYPE>>*>(in_input);
			if (nullptr != value_unique_node)
			{
				std::vector<IN_TYPE>* array = value_unique_node->GetValue(true);
				array->erase(std::remove(array->begin(), array->end(), in_input), array->end());
				return;
			}

			DSC_ASSERT_ALWAYS("invalid code path");
			return;
		}

		//pop
		//set
		template <typename IN_TYPE>
		static void Set(NodeToken in_input, const int32 in_index, const IN_TYPE& in_value, std::function<void(bool& out_dirty, bool& out_bail, const IN_TYPE&, const IN_TYPE& in_rhs)>& in_on_set_callback)
		{
			DSC_ASSERT(nullptr != in_input, "invalid param");
			//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "GetArraySize in_input:%s IN_TYPE:%s\n", in_input->GetTypeInfo().name(), typeid(std::vector<IN_TYPE>).name());
			DSC_ASSERT(typeid(std::vector<IN_TYPE>) == in_input->GetTypeInfo(), "invalid param");

			auto value_node = dynamic_cast<DagNodeValue<std::vector<IN_TYPE>>*>(in_input);
			if (nullptr != value_node)
			{
				bool set_dirty = false;
				bool bail = false;
				//IN_CALLBACK::Function(set_dirty, bail, _value, in_value);
				in_on_set_callback(set_dirty, bail, value_node->GetValue()[in_index], in_value);
				if (true == bail)
				{
					return;
				}

				value_node->GetValueNonConst(set_dirty)[in_index] = in_value;
			}

			auto value_unique_node = dynamic_cast<DagNodeValueUnique<std::vector<IN_TYPE>>*>(in_input);
			if (nullptr != value_unique_node)
			{
				bool set_dirty = false;
				bool bail = false;
				//IN_CALLBACK::Function(set_dirty, bail, _value, in_value);
				in_on_set_callback(set_dirty, bail, value_unique_node->GetValue()[in_index], in_value);
				if (true == bail)
				{
					return;
				}

				value_unique_node->GetValueNonConst(set_dirty)[in_index] = in_value;
			}

			DSC_ASSERT_ALWAYS("invalid code path");
			return;
		}
		//resize


	} // ArrayHelper


} //DscDag
