#pragma once
#include "dsc_dag.h"
#include "i_dag_node.h"
#include <dsc_common\dsc_common.h>

namespace DscDag
{
	class DagCollection;

	/// when condition is calculated as true, we take the value of in_true_source and set in_true_destination with it
	/// when condition is calculated as false, we take the value of in_false_source and set in_false_destination with it
	/// if not null, we add ourself to the output of in_condition, in_true_source, in_false_source
	/// when we are marked dirty, we add ourself to a DagCollection::AddDirtyConditional
	///
	/// we don't want to dirty our output chain on input dirty as that may be unneccessary, ie, we act as a filter on the dirty flag flow
	/// we can however accomidate source dirty flag and treat ourself as dirty on them being set, as well as the actual condition node
	class DagNodeCondition: public IDagNode
	{
	public:
		DagNodeCondition() = delete;
		DagNodeCondition& operator=(const DagNodeCondition&) = delete;
		DagNodeCondition(const DagNodeCondition&) = delete;

		DagNodeCondition(
			DagCollection& in_dag_collection,
			//NodeToken in_condition, // use SetIndexInput(0, in_condition)
			NodeToken in_true_source,
			NodeToken in_false_source,
			NodeToken in_true_destination, 
			NodeToken in_false_destination
			DSC_DEBUG_ONLY(DSC_COMMA const std::string & in_debug_name = "")
		);


	private:
		// used to trigger calculation? return true/ false (if we have a condition node attached)
		virtual void MarkDirty() override;
		// if dirty, flush dirty flags from input, check condition, set appropriate output
		virtual void Update() override;
		virtual const bool GetHasNoLinks() const override;
		virtual const bool SetIndexInput(const int32 in_index, NodeToken in_nodeID = NullToken) override;
		virtual void AddInput(NodeToken in_nodeID) override;
		virtual void RemoveInput(NodeToken in_nodeID) override;
		virtual void UnlinkInputs() override;
		virtual const std::type_info& GetTypeInfo() const override;

#if defined(_DEBUG)
		virtual const std::string DebugPrint(const int32 in_depth = 0) const override;
#endif //#if defined(_DEBUG)

	private:
		DagCollection& _dag_collection;
		bool _dirty = false;
		// cache the condition result, also indicates that we have linked ourself to the true or false source
		bool _condition_true = false;
		bool _condition_false = false;
		NodeToken _condition = nullptr; // expect to be set via SetIndexInput
		NodeToken _true_source = nullptr;
		NodeToken _false_source = nullptr;
		NodeToken _true_destination = nullptr;
		NodeToken _false_destination = nullptr;

		// if we start linking in_true_source and in_false_souce as inputs when they match our condition state
		std::set<NodeToken> _input = {};

	}; // DagNodeValue
} //DscDag
