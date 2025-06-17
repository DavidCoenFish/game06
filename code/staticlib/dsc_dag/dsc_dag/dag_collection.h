#pragma once
#include "dsc_dag.h"
#include <dsc_common\dsc_common.h>
#include <dsc_dag\i_dag_node.h>

namespace DscDag
{
	typedef std::function<void(std::any&, std::set<NodeToken>&, std::vector<NodeToken>&)> TCalculateFunction;

	class DagCollection
	{
	public:
		NodeToken CreateValue(const std::any& in_value);
		NodeToken CreateCalculate(const TCalculateFunction& in_calculate);
		// was trying to shoe horn generic values into a std::any for CreateValue, but std::any can not hold a std::unique_ptr
		NodeToken AddCustomNode(std::unique_ptr<IDagNode>&& in_node);
		// should already have all links removed? assert if links still exisit?
		void DeleteNode(NodeToken in_node);

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

		std::set<std::unique_ptr<IDagNode>, RawPtrComparator> _nodes = {};
	}; // IDagNode
} //DscDag
