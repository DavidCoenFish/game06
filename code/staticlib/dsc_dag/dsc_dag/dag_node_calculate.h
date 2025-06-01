#include <dsc_common\dsc_common.h>

namespace DscDag
{
	typedef void* NodeToken;
	constexpr NodeToken NullToken = nullptr;

	class IDagNode
	{
	public:
		virtual ~IDagNode() {}

		//assert on value node
		virtual void MarkDirty() = 0;
		virtual void AddOutput(NodeToken in_nodeID) = 0;
		virtual void RemoveOutput(NodeToken in_nodeID) = 0;
		virtual void SetIndexInput(const int32 in_index, NodeToken in_nodeID = NullToken) = 0;
		virtual void AddInput(NodeToken in_nodeID) = 0;
		virtual void RemoveInput(NodeToken in_nodeID) = 0;
		virtual void SetValue(const std::any& value) = 0;
		// not const as calculate may trigger state change
		virtual const std::any& GetValue() = 0;

	}; // IDagNode
} //DscDag
