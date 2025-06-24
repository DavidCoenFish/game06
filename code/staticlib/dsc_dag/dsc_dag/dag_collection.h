#pragma once
#include "dsc_dag.h"
#include <dsc_common\dsc_common.h>
#include <dsc_dag\i_dag_node.h>
#include <dsc_dag\dag_enum.h>

namespace DscDag
{
	typedef std::function<void(std::any&, std::set<NodeToken>&, std::vector<NodeToken>&)> TCalculateFunction;
	class IDagNode;
	class DagNodeCondition;

	class DagCollection
	{
	public:
		NodeToken CreateValue(const std::any& in_value, const TValueChangeCondition in_change_condition = TValueChangeCondition::TOnValueChange  DSC_DEBUG_ONLY(DSC_COMMA const std::string & in_debug_name = ""));
		NodeToken CreateCalculate(const TCalculateFunction& in_calculate  DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = ""));
		// note. we expect index input 0 to be a true/ false node, index input 1 to be true source value, and index input 2 to be false source
		// these can be set latter, for convienience, they are param here and automatically linked
		// condition doesn't have a normal link to destination node, they are only set when the condition is updated/ resolved. see ResolveDirtyConditionNodes
		NodeToken CreateCondition(NodeToken in_condition, NodeToken in_true_source, NodeToken in_false_source, NodeToken in_true_destination, NodeToken in_false_destination DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = ""));

		// was trying to shoe horn generic values into a std::any for CreateValue, but std::any can not hold a std::unique_ptr. so allow custom IDagNode to hold things like std::unique
		// todo, write a generic DagNodeValueUnique?
		NodeToken AddCustomNode(std::unique_ptr<IDagNode>&& in_node);
		// should already have all links removed? assert if links still exisit?
		void DeleteNode(NodeToken in_node);

		void AddDirtyConditionNode(NodeToken in_dirty_condition_node);
		// keep on resolving dirty condition nodes till there are no more
		void ResolveDirtyConditionNodes();

		void AddNodeName(NodeToken in_node, const std::string& in_name);
		void RemoveNodeName(const std::string& in_name);
		NodeToken FetchNodeName(const std::string& in_name);

		static void LinkNodes(NodeToken in_input, NodeToken in_output);
		static void LinkIndexNodes(int32 in_index, NodeToken in_input, NodeToken in_output);

		static void UnlinkNodes(NodeToken in_input, NodeToken in_output);
		static void UnlinkIndexNodes(int32 in_index, NodeToken in_input, NodeToken in_output);
		
		static const std::any& GetValue(NodeToken in_input);
		static void SetValue(NodeToken in_input, const std::any& in_value);

		template <typename TYPE>
		static const TYPE GetValueType(NodeToken in_input)
		{
			auto& any = GetValue(in_input);
			if (typeid(TYPE) == any.type())
			{
				return std::any_cast<TYPE>(any);
			}
			static TYPE kData = {};
			return kData;
		}

		template <typename TYPE>
		static void SetValueType(NodeToken in_input, const TYPE in_value)
		{
			SetValue(in_input, std::any(in_value));
		}
		template <typename TYPE>
		static void SetValueTypeRef(NodeToken in_input, const TYPE& in_value)
		{
			SetValue(in_input, std::any(in_value));
		}

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
