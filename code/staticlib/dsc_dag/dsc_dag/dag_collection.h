#pragma once
#include "dsc_dag.h"
#include <dsc_common\dsc_common.h>
#include <dsc_common\log_system.h>
#include "dag_node_array.h"
#include "dag_node_calculate.h"
#include "dag_node_condition.h"
#include "dag_node_value.h"
#include "i_dag_owner.h"
//#include "dag_node_group.h"

namespace DscDag
{
	class IDagNode;
	class IDagOwner;

	class DagCollection
	{
	public:
		//CreateArray

		template <typename IN_TYPE>
		NodeToken CreateCalculate(
			const typename DagNodeCalculate<IN_TYPE>::TCalculateFunction& in_calculate,
			IDagOwner* in_dag_owner_or_nullptr = nullptr
		)
		{
			auto node = std::make_unique<DagNodeCalculate<IN_TYPE>>(in_calculate);
			NodeToken node_token = node.get();
			_nodes.insert(std::move(node));
			if (nullptr != in_dag_owner_or_nullptr)
			{
				in_dag_owner_or_nullptr->AddOwnership(node_token);
			}
			return node_token;
		}

		/// the in_true_source and in_false_source are only linked to the dirty path of the destination when the condition has calculated as true or false
		/// THERE is a small window when condition is wating to change, that the non logically correct source and destination can still be linked
		/// condition doesn't have a normal link to destination node, they are only set when the condition is updated/ resolved. see ResolveDirtyConditionNodes
		template <typename IN_TYPE_TRUE, typename IN_TYPE_FALSE>
		NodeToken CreateCondition(
			NodeToken in_condition,
			NodeToken in_true_source,
			NodeToken in_false_source,
			NodeToken in_true_destination,
			NodeToken in_false_destination,
			IDagOwner* in_dag_owner_or_nullptr = nullptr
		)
		{
			auto node = std::make_unique<DagNodeCondition<IN_TYPE_TRUE, IN_TYPE_FALSE>>(
				in_condition,
				in_true_source,
				in_false_source,
				in_true_destination,
				in_false_destination
				);
			NodeToken node_token = node.get();
			_nodes.insert(std::move(node));
			if (nullptr != in_dag_owner_or_nullptr)
			{
				in_dag_owner_or_nullptr->AddOwnership(node_token);
			}
			return node_token;
		}

		//create group

		template <typename IN_TYPE>
		NodeToken CreateValue(
			const IN_TYPE& in_value,
			typename const DscDag::DagNodeValue<IN_TYPE>::TCallbackOnSet& in_on_set_callback = typename DscDag::CallbackOnValueChange<IN_TYPE>::Function,
			IDagOwner* in_dag_owner_or_nullptr = nullptr
		)
		{
			auto node = std::make_unique<DagNodeValue<IN_TYPE>>(in_value, in_on_set_callback);
			NodeToken node_token = node.get();
			_nodes.insert(std::move(node));
			if (nullptr != in_dag_owner_or_nullptr)
			{
				in_dag_owner_or_nullptr->AddOwnership(node_token);
			}
			return node_token;
		}



		// should already have all links removed? assert if links still exisit?
		void DeleteNode(NodeToken in_node);

		void AddDirtyConditionNode(NodeToken in_dirty_condition_node);
		// keep on resolving dirty condition nodes till there are no more. currious about moving this into DagGroup::ResolveDirtyConditionNodes() and have it reference it's DagCollection
		void ResolveDirtyConditionNodes();

		// allow giving nodes a lookup name/ label
		void AddNodeName(NodeToken in_node, const std::string& in_name);
		void RemoveNodeName(const std::string& in_name);
		NodeToken FetchNodeName(const std::string& in_name);

		const int32 GetNodeCount() const;

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
