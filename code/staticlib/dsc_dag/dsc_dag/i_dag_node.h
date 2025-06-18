#pragma once
#include <dsc_common\dsc_common.h>

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;
	constexpr NodeToken NullToken = nullptr;

	class IDagNode
	{
	public:
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
		//assert on custom node
		virtual const std::any& GetValue();

	}; // IDagNode
} //DscDag
