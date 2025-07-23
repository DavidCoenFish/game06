#pragma once
#include "dsc_dag.h"
#include "i_dag_node.h"

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;

	/// we hold a node token
	class DagNodeNode : public IDagNode
	{
	public:
		DagNodeNode(NodeToken in_node_or_null);

		NodeToken GetValue() const;
		void SetValue(NodeToken in_node_or_null);

	private:
		virtual void MarkDirty() override;
		virtual void Update() override;
		virtual void AddOutput(NodeToken in_node) override;
		virtual void RemoveOutput(NodeToken in_node) override;
		virtual const bool GetHasNoLinks() const override;
		virtual void UnlinkInputs() override;

#if defined(_DEBUG)
		virtual const std::type_info& DebugGetTypeInfo() const override;
		virtual const std::string DebugPrintRecurseInputs(const int32 in_depth = 0) const override;
		virtual const std::string DebugPrintRecurseOutputs(const int32 in_depth = 0) const override;
#endif //#if defined(_DEBUG)

	private:
		NodeToken _node = {};
		std::set<NodeToken> _output = {};
		bool _dirty = false;

	}; // DagGroup
} //DscDag
