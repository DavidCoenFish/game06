#pragma once
#include "dsc_dag.h"
#include "i_dag_node.h"
#include "accessor.h"
#include "link.h"
#include <dsc_common\dsc_common.h>
#include <dsc_common\debug_print.h>

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
	
	template <typename IN_TYPE_TRUE, typename IN_TYPE_FALSE>
	class DagNodeCondition : public IDagNode
	{
	public:
		DagNodeCondition() = delete;
		DagNodeCondition& operator=(const DagNodeCondition&) = delete;
		DagNodeCondition(const DagNodeCondition&) = delete;

		DagNodeCondition(
			DagCollection& in_dag_collection,
			NodeToken in_condition,
			NodeToken in_true_source_or_null,
			NodeToken in_false_source_or_null,
			NodeToken in_true_destination_or_null,
			NodeToken in_false_destination_or_null
		)
			: _dag_collection(in_dag_collection)
			, _condition(in_condition)
			, _true_source(in_true_source_or_null)
			, _false_source(in_false_source_or_null)
			, _true_destination(in_true_destination_or_null)
			, _false_destination(in_false_destination_or_null)
		{
			DSC_ASSERT(nullptr != in_condition, "invalid param");
			if (nullptr != _condition)
			{
				_condition->AddOutput(this);
			}
		}

	private:
		virtual void MarkDirty() override
		{
			if (false == _dirty)
			{
				_dirty = true;
				_dag_collection.AddDirtyConditionNode(this);

				for (auto& item : _output)
				{
					item->MarkDirty();
				}
			}

			return;
		}
		virtual void Update() override
		{
			if (true == _dirty)
			{
				if (nullptr == _condition)
				{
					_dirty = false;
					return;
				}

				// update the input links, this is an attempt to not have the conditional node dirtied from a non active branch
				if (true == GetValueType<bool>(_condition))
				{
					if (true != _true_linked)
					{
						_true_linked = true;
						if (nullptr != _true_source)
						{
							_true_source->AddOutput(this);
						}

						if (true == _false_linked)
						{
							if (nullptr != _false_source)
							{
								_false_source->RemoveOutput(this);
							}
							_false_linked = false;
						}
					}

					if ((nullptr != _true_source) &&
						(nullptr != _true_destination))
					{
						SetValueType< IN_TYPE_TRUE>(
							_true_destination,
							GetValueType<IN_TYPE_TRUE>(_true_source)
							);
					}
				}
				else
				{
					if (true != _false_linked)
					{
						_false_linked = true;
						if (nullptr != _false_source)
						{
							_false_source->AddOutput(this);
						}

						if (true == _true_linked)
						{
							if (nullptr != _true_source)
							{
								_true_source->RemoveOutput(this);
							}
							_true_linked = false;
						}
					}

					if ((nullptr != _true_source) &&
						(nullptr != _true_destination))
					{
						SetValueType<IN_TYPE_FALSE>(
							_false_destination,
							GetValueType<IN_TYPE_FALSE>(_false_source)
							);
					}
				}

				// linking/ unlinking the [true/false]source to us will dirty us, so defer unmarking dirty flag till AFTER linking source
				//DSC_ASSERT(false == _dirty, "Cyclic conditional dirty");
				_dirty = false;
			}

			return;
		}
		virtual void AddOutput(NodeToken in_node) override
		{
			DSC_ASSERT(nullptr != in_node, "invalid param");
			if (nullptr != in_node)
			{
				in_node->MarkDirty();
			}
			_output.insert(in_node);
		}

		virtual void RemoveOutput(NodeToken in_node) override
		{
			DSC_ASSERT(nullptr != in_node, "invalid param");
			if (nullptr != in_node)
			{
				in_node->MarkDirty();
			}
			_output.erase(in_node);
		}

		virtual const bool GetHasNoLinks() const override
		{
			if (0 != _output.size())
			{
				return false;
			}
			if (true == _true_linked)
			{
				return false;
			}
			if (true == _false_linked)
			{
				return false;
			}
			return _unlinked;
		}

		virtual void UnlinkInputs() override
		{
			if (false == _unlinked)
			{
				if (nullptr != _condition)
				{
					_condition->UnlinkInputs();
					_condition->RemoveOutput(this);
					//_condition = nullptr;
				}
				if (true == _true_linked)
				{
					if (nullptr != _true_source)
					{
						_true_source->UnlinkInputs();
						_true_source->RemoveOutput(this);
						//UnlinkNodes(_true_source, _true_destination);
					}
					_true_linked = false;
				}
				if (true == _false_linked)
				{
					if (nullptr != _false_source)
					{
						_false_source->UnlinkInputs();
						_false_source->RemoveOutput(this);
						//UnlinkNodes(_false_source, _false_destination);
					}
					_false_linked = false;
				}
				_unlinked = true;
			}
			return;
		}

#if defined(_DEBUG)
		// only the True branch type..? or just return bool type, we don't directly return a value, we get and set other node values based on condition
		virtual const std::type_info& DebugGetTypeInfo() const override
		{
			return typeid(bool);
		}

		virtual const std::string DebugPrintRecurseInputs(const int32 in_depth = 0) const override
		{
			std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

			result += "Condition:\"";
			result += DebugGetNodeName();
			result += "\" dirty:" + std::to_string(_dirty);
			result += "\n";

			if (nullptr != _condition)
			{
				result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
				result += "condition:\n";
				result += _condition->DebugPrintRecurseInputs(in_depth + 2);
			}

			if (nullptr != _true_source)
			{
				result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
				result += "true_source:\n";
				result += _true_source->DebugPrintRecurseInputs(in_depth + 2);
			}

			if (nullptr != _false_source)
			{
				result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
				result += "false_source:\n";
				result += _false_source->DebugPrintRecurseInputs(in_depth + 2);
			}

			return result;
		}
		virtual const std::string DebugPrintRecurseOutputs(const int32 in_depth = 0) const override
		{
			std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

			result += "Condition:\"";
			result += DebugGetNodeName();
			result += "\" dirty:" + std::to_string(_dirty);
			result += "\n";

			if (nullptr != _true_destination)
			{
				result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
				result += "true_destination:\n";
				result += _true_destination->DebugPrintRecurseOutputs(in_depth + 2);
			}

			if (nullptr != _false_destination)
			{
				result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
				result += "false_destination:\n";
				result += _false_destination->DebugPrintRecurseOutputs(in_depth + 2);
			}

			if (0 < _output.size())
			{
				result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
				result += "output:\n";
				for (NodeToken item : _output)
				{
					if (nullptr != item)
					{
						result += item->DebugPrintRecurseOutputs(in_depth + 2);
					}
				}
			}

			return result;
		}
#endif //#if defined(_DEBUG)

	private:
		DagCollection& _dag_collection;
		bool _dirty = false;
		// once we are unlinked, assert if there is a call to update
		bool _unlinked = false;
		bool _true_linked = false;
		bool _false_linked = false;
		NodeToken _condition = nullptr; // input
		NodeToken _true_source = nullptr; // input mostly when condition is true, state can be stick (delay of calculating condition)
		NodeToken _false_source = nullptr; // input mostly when condition is false, state can be stick (delay of calculating condition)
		NodeToken _true_destination = nullptr;
		NodeToken _false_destination = nullptr;

		// if we are held by a DagNodeGroup or DagNodeArray, then we can have output, even if they just want to chain to our dirty state
		std::set<NodeToken> _output = {};

	}; // DagNodeValue
} //DscDag
