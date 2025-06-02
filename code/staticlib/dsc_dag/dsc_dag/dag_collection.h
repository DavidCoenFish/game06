#pragma once
#include <dsc_common\dsc_common.h>
#include <dsc_dag\i_dag_node.h>

namespace DscDag
{
	typedef std::function<void(std::any&, std::set<NodeToken>, std::vector<NodeToken>)> TCalculateFunction;

	class DagCollection
	{
	public:
		NodeToken CreateValue(std::any& in_value);
		NodeToken CreateCalculate(const TCalculateFunction& in_calculate);

		void LinkNodes(NodeToken in_input, NodeToken in_output);
		void LinkIndexNodes(int32 in_index, NodeToken in_input, NodeToken in_output);

		void UnlinkNodes(NodeToken in_input, NodeToken in_output);
		void UnlinkIndexNodes(int32 in_index, NodeToken in_input, NodeToken in_output);

		// don't wnat to keep passing around a DagCollection just to get value from nodes?
		//std::any& GetValue(NodeToken in_input);
		//void SetValue(NodeToken in_input, std::any& in_value);

	private:
		std::set<std::unique_ptr<IDagNode>> _nodes = {};
	}; // IDagNode
} //DscDag
