#pragma once
#include "dsc_dag.h"
#include "i_dag_node.h"

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;

	/// we hold an array of node tokens, as they are added to our array, we set ourself as their output, and when they are removed, we remove outselves from their output
	class DagNodeArray : public IDagNode
	{
	public:
		void PushBack(NodeToken in_node);
		void Remove(NodeToken in_node);
		NodeToken GetAt(const int32 in_index) const;
		void SetAt(NodeToken in_node, const int32 in_index);
		const int32 GetCount() const;

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
		std::vector<NodeToken> _node_array = {};

	}; // DagGroup
} //DscDag
