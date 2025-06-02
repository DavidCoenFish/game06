#pragma once
#include <dsc_common\dsc_common.h>
#include <dsc_dag\i_dag_node.h>

namespace DscDag
{
	typedef std::function<void(std::any&, std::set<NodeToken>, std::vector<NodeToken>)> TCalculateFunction;

	class DagNodeCalculate : public IDagNode
	{
	public:
		DagNodeCalculate() = delete;
		DagNodeCalculate& operator=(const DagNodeCalculate&) = delete;
		DagNodeCalculate(const DagNodeCalculate&) = delete;

		DagNodeCalculate(const TCalculateFunction& in_calculateFunction);

	private:
		virtual void MarkDirty() override;
		virtual void AddOutput(NodeToken in_nodeID) override;
		virtual void RemoveOutput(NodeToken in_nodeID) override;
		virtual void SetIndexInput(const int32 in_index, NodeToken in_nodeID = NullToken) override;
		virtual void AddInput(NodeToken in_nodeID) override;
		virtual void RemoveInput(NodeToken in_nodeID) override;
		virtual const std::any& GetValue() override;

	private:
		bool _dirty = true;
		const TCalculateFunction _calculateFunction = {};
		std::any _value = {};

		std::set<NodeToken> _input = {};
		std::vector<NodeToken> _indexInput = {};
		std::set<NodeToken> _output = {};

	}; // DagNodeCalculate
} //DscDag
