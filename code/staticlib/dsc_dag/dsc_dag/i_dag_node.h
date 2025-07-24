#pragma once
#include "dsc_dag.h"

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;
	//constexpr NodeToken NullToken = nullptr;

	class IDagNode
	{
	public:
		IDagNode() {}
		virtual ~IDagNode() {}

		// mare ourself as dirty, if we were not already dirty, mark our outputs as dirty as well
		virtual void MarkDirty();
		// if dirty, then tell inputs to update, and then calculate (if a calculate node)
		virtual void Update();
		virtual void AddOutput(NodeToken in_node) = 0;
		virtual void RemoveOutput(NodeToken in_node) = 0;
		virtual const bool GetHasNoLinks() const;
		virtual void UnlinkInputs();

#if defined(_DEBUG)
		virtual const std::type_info& DebugGetTypeInfo() const = 0;
		virtual const std::string DebugPrintRecurseInputs(const int32 in_depth = 0) const = 0;
		virtual const std::string DebugPrintRecurseOutputs(const int32 in_depth = 0) const = 0;
#endif //#if defined(_DEBUG)

#if defined(_DEBUG)
		void DebugSetNodeName(const std::string& in_debug_name);
		const std::string& DebugGetNodeName() const {
			return _debug_name;
		}
#endif //#if defined(_DEBUG)

	private:
		DSC_DEBUG_ONLY(std::string _debug_name);

	}; // IDagNode
} //DscDag
