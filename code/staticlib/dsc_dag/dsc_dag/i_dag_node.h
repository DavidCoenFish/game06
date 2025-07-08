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

		//assert on value node, they have their own logic for setting dirty?
		virtual void MarkDirty();
		// regenerate value/ remove dirty flag as a side effect?
		virtual void Update();
		virtual void SetIndexInput(const int32 in_index, NodeToken in_nodeID = NullToken);
		virtual void AddInput(NodeToken in_nodeID);
		virtual void RemoveInput(NodeToken in_nodeID);
		virtual void AddOutput(NodeToken in_nodeID);
		virtual void RemoveOutput(NodeToken in_nodeID);

		virtual const bool GetHasNoLinks() const = 0;

		virtual void UnlinkInputs();

		virtual const std::type_info& GetTypeInfo() const = 0;

		// for DagCondition, wanted to be able to assign output of one node to another node, but with type infor kept in the derrived classes
		// asserts when called if not overridden 
		virtual void SetFromNode(IDagNode* const in_node);

#if defined(_DEBUG)
		virtual const std::string DebugPrint(const int32 in_depth = 0) const = 0;
#endif //#if defined(_DEBUG)

	protected:
		DSC_DEBUG_ONLY(std::string _debug_name);

	}; // IDagNode
} //DscDag
