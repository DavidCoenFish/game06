#pragma once
#include "dsc_dag.h"

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;
	constexpr NodeToken NullToken = nullptr;

	class IDagNode
	{
	public:
		IDagNode(DSC_DEBUG_ONLY(const std::string& in_debug_name));
		virtual ~IDagNode();

		//assert on value node
		virtual void MarkDirty();

		virtual const bool GetHasNoLinks() const = 0;

		virtual void AddOutput(NodeToken in_nodeID);
		virtual void RemoveOutput(NodeToken in_nodeID);
		virtual void SetIndexInput(const int32 in_index, NodeToken in_nodeID = NullToken);
		virtual NodeToken GetIndexInput(const int32 in_index) const;
		virtual void AddInput(NodeToken in_nodeID);
		virtual void RemoveInput(NodeToken in_nodeID);

		//assert on calculate node or custom node
		virtual void SetValue(const std::any& in_value);

		//not const as calculate may trigger state change
		// even on custom nodes this needs to be implemented, as now Calculate trigger GetValue on input to purge the input dirty flag
		// this is starting to be used to trigger calculation/ evaluate condition, the returned std::any may actually just be empty for those cases
		virtual const std::any& GetValue() = 0;

	private:
		DSC_DEBUG_ONLY(std::string _debug_name);

	}; // IDagNode
} //DscDag
