#pragma once
#include "dsc_dag.h"
#include <dsc_common\dsc_common.h>
#include <dsc_dag\i_dag_node.h>
#include <dsc_dag\dag_enum.h>
#include <dsc_dag\dag_node_value.h>
#include <dsc_dag\dag_node_value_unique.h>
#include <dsc_dag\dag_node_calculate.h>

namespace DscDag
{
	class IDagNode;
	class DagNodeCondition;

	class DagCollection
	{
	public:
		template <typename IN_TYPE, typename IN_CALLBACK = DscDag::CallbackOnValueChange<IN_TYPE>>
		NodeToken CreateValue(const IN_TYPE& in_value DSC_DEBUG_ONLY(DSC_COMMA const std::string & in_debug_name = ""))
		{
			auto node = std::make_unique<DagNodeValue<IN_TYPE, IN_CALLBACK>>(in_value DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
			NodeToken node_token = node.get();
			_nodes.insert(std::move(node));
			return node_token;
		}

		template <typename IN_TYPE>
		NodeToken CreateValueUnique(std::unique_ptr<IN_TYPE>&& in_value DSC_DEBUG_ONLY(DSC_COMMA const std::string & in_debug_name = ""))
		{
			auto node = std::make_unique<DagNodeValueUnique<IN_TYPE>>(std::move(in_value) DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
			NodeToken node_token = node.get();
			_nodes.insert(std::move(node));
			return node_token;
		}

		template <typename IN_TYPE>
		//NodeToken CreateCalculate(const std::function<void(IN_TYPE&, std::set<NodeToken>&, std::vector<NodeToken>&)>& in_calculate  DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = ""))
		NodeToken CreateCalculate(const typename DagNodeCalculate<IN_TYPE>::TCalculateFunction& in_calculate  DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = ""))
		{
			auto node = std::make_unique<DagNodeCalculate<IN_TYPE>>(in_calculate DSC_DEBUG_ONLY(DSC_COMMA in_debug_name));
			NodeToken node_token = node.get();
			_nodes.insert(std::move(node));
			return node_token;
		}

		// note. we expect index input 0 to be a true/ false node, index input 1 to be true source value, and index input 2 to be false source
		// these can be set latter, for convienience, they are param here and automatically linked
		// condition doesn't have a normal link to destination node, they are only set when the condition is updated/ resolved. see ResolveDirtyConditionNodes
		NodeToken CreateCondition(NodeToken in_condition, NodeToken in_true_source, NodeToken in_false_source, NodeToken in_true_destination, NodeToken in_false_destination DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = ""));

		// should already have all links removed? assert if links still exisit?
		void DeleteNode(NodeToken in_node);

		void AddDirtyConditionNode(NodeToken in_dirty_condition_node);
		// keep on resolving dirty condition nodes till there are no more. currious about moving this into DagGroup::ResolveDirtyConditionNodes() and have it reference it's DagCollection
		void ResolveDirtyConditionNodes();

		void AddNodeName(NodeToken in_node, const std::string& in_name);
		void RemoveNodeName(const std::string& in_name);
		NodeToken FetchNodeName(const std::string& in_name);

		static void LinkNodes(NodeToken in_input, NodeToken in_output);
		static void LinkIndexNodes(int32 in_index, NodeToken in_input, NodeToken in_output);

		static void UnlinkNodes(NodeToken in_input, NodeToken in_output);
		static void UnlinkIndexNodes(int32 in_index, NodeToken in_input, NodeToken in_output);
		
		template <typename IN_TYPE>
		static const IN_TYPE& GetValueType(NodeToken in_input)
		{
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

			auto value_unique_node = dynamic_cast<DagNodeValueUnique< IN_TYPE>*>(in_input);
			if (nullptr != value_unique_node)
			{
				return *value_unique_node->GetValue();
			}

			DSC_ASSERT_ALWAYS("invalid code path");
			static IN_TYPE kData = {};
			return kData;
		}

		template <typename IN_TYPE>
		static IN_TYPE* GetUniqueValueType(NodeToken in_input)
		{
			auto value_unique_node = dynamic_cast<DagNodeValueUnique< IN_TYPE>*>(in_input);
			if (nullptr != value_unique_node)
			{
				return value_unique_node->GetValue();
			}

			DSC_ASSERT_ALWAYS("invalid code path");
			return nullptr;
		}

		template <typename IN_TYPE>
		static void SetValueType(NodeToken in_input, const IN_TYPE& in_value)
		{
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

#if defined(_DEBUG)
		static void DebugDumpNode(NodeToken in_input);
#endif //#if defined(_DEBUG)

	private:
		struct RawPtrComparator {
			bool operator()(const std::unique_ptr<DscDag::IDagNode>& a, const std::unique_ptr<DscDag::IDagNode>& b) const;
		};

		// ownership of nodes
		std::set<std::unique_ptr<IDagNode>, RawPtrComparator> _nodes = {};

		// map of named nodes
		std::map<std::string, NodeToken> _named_nodes = {};

		// the set of dirty condition nodes
		std::set<NodeToken> _dirty_condition_nodes = {};

	}; // IDagNode
} //DscDag
