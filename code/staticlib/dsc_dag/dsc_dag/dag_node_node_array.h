#pragma once
#include "dsc_dag.h"
#include "i_dag_node.h"

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;

	/// we hold an array of node tokens, as they are added to our array, we set ourself as their output, and when they are removed, we remove outselves from their output
	/// needs to preserve order, so not a std::set (ui child array, usage draw order)
	class DagNodeNodeArray : public IDagNode
	{
	public:
		DagNodeNodeArray(const std::vector<NodeToken>& in_node_array);

		void PushBack(NodeToken in_node);
		void Remove(NodeToken in_node);
		NodeToken GetAt(const int32 in_index) const;
		void SetAt(NodeToken in_node_or_null, const int32 in_index);
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
		std::set<NodeToken> _output = {};
		bool _dirty = false;

	}; // DagGroup
} //DscDag
