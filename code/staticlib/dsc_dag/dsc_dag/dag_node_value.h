#pragma once
#include <dsc_common\dsc_common.h>
#include <dsc_dag\dag_enum.h>

namespace DscDag
{
	class DagNodeValue : public IDagNode
	{
	public:
		DagNodeValue(const std::any& in_value, const TValueChangeCondition in_change_condition = TValueChangeCondition::TOnValueChange);

	private:
		virtual void AddOutput(NodeToken in_nodeID) override;
		virtual void RemoveOutput(NodeToken in_nodeID) override;

		virtual void SetValue(const std::any& in_value) override;
		// not const as calculate may trigger state change
		virtual const std::any& GetValue() override;

		virtual const bool GetHasNoLinks() const override;

	private:
		TValueChangeCondition _change_condition = {};
		std::any _value = {};
		std::set<NodeToken> _output = {};

	}; // DagNodeValue
} //DscDag
