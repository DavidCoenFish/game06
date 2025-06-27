#pragma once
#include "dsc_dag.h"
#include <dsc_dag\i_dag_node.h>

namespace DscDag
{
	typedef std::function<void(std::any&, std::set<NodeToken>&, std::vector<NodeToken>&)> TCalculateFunction;

	class DagNodeCalculate : public IDagNode
	{
	public:
		DagNodeCalculate() = delete;
		DagNodeCalculate& operator=(const DagNodeCalculate&) = delete;
		DagNodeCalculate(const DagNodeCalculate&) = delete;

		DagNodeCalculate(const TCalculateFunction& in_calculateFunction DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = ""));

	private:
		virtual void MarkDirty() override;
		virtual void AddOutput(NodeToken in_nodeID) override;
		virtual void RemoveOutput(NodeToken in_nodeID) override;
		virtual void SetIndexInput(const int32 in_index, NodeToken in_nodeID = NullToken) override;
		//virtual NodeToken GetIndexInput(const int32 in_index) const override;
		virtual void AddInput(NodeToken in_nodeID) override;
		virtual void RemoveInput(NodeToken in_nodeID) override;
		virtual const std::any& GetValue() override;
		virtual const bool GetHasNoLinks() const override;

#if defined(_DEBUG)
		virtual const std::string DebugPrint(const int32 in_depth = 0) const override;
#endif //#if defined(_DEBUG)

	private:
		bool _dirty = true;
		const TCalculateFunction _calculateFunction = {};
		std::any _value = {};

		std::set<NodeToken> _input = {};
		std::vector<NodeToken> _indexInput = {};
		std::set<NodeToken> _output = {};

	}; // DagNodeCalculate
} //DscDag
