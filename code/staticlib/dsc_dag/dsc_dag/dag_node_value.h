#pragma once
#include <dsc_common\dsc_common.h>
#include <dsc_dag\i_dag_node.h>

namespace DscDag
{
	class DagNodeValue : public IDagNode
	{
	public:
		DagNodeValue(const std::any& in_value);

	private:
		virtual void AddOutput(NodeToken in_nodeID) override;
		virtual void RemoveOutput(NodeToken in_nodeID) override;

		virtual void SetValue(const std::any& in_value) override;
		// not const as calculate may trigger state change
		virtual const std::any& GetValue() override;

	private:
		std::any _value = {};
		std::set<NodeToken> _output = {};

	}; // DagNodeValue
} //DscDag
