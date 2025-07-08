#include "dag_node_condition.h"
#include "dag_collection.h"

namespace
{
}

DscDag::DagNodeCondition::DagNodeCondition(
	DagCollection& in_dag_collection,
	NodeToken in_true_source,
	NodeToken in_false_source,
	NodeToken in_true_destination,
	NodeToken in_false_destination
	DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
)
	: IDagNode(DSC_DEBUG_ONLY(in_debug_name))
	, _dag_collection(in_dag_collection)
	, _true_source(in_true_source)
	, _false_source(in_false_source)
	, _true_destination(in_true_destination)
	, _false_destination(in_false_destination)
{
	//nop
}

void DscDag::DagNodeCondition::Update()
{
	if (true == _dirty)
	{
		if (nullptr == _condition)
		{
			_dirty = false;
			return;
		}

		// update the input links, this is an attempt to not have the conditional node dirtied from a non active branch
		if (true == DagCollection::GetValueType<bool>(_condition))
		{
			if (true != _condition_true)
			{
				_condition_true = true;
				if (nullptr != _true_source)
				{
					DagCollection::LinkNodes(_true_source, this);
				}

				if ((true == _condition_false) && (nullptr != _false_source))
				{
					DagCollection::UnlinkNodes(_false_source, this);
				}
			}

			if ((nullptr != _true_source) && (nullptr != _true_destination))
			{
				_true_destination->SetFromNode(_true_source);
			}
		}
		else
		{
			if (true != _condition_false)
			{
				_condition_false = true;
				if (nullptr != _false_source)
				{
					DagCollection::LinkNodes(_false_source, this);
				}

				if ((true == _condition_true) && (nullptr != _true_source))
				{
					DagCollection::UnlinkNodes(_true_source, this);
				}
			}

			if ((nullptr != _false_source) && (nullptr != _false_destination))
			{
				_false_destination->SetFromNode(_false_source);
			}
		}

		// linking/ unlinking the source to us will dirty us, so defer unmarking dirty flag till AFTER linking source
		//DSC_ASSERT(false == _dirty, "Cyclic conditional dirty");
		_dirty = false;
	}

	return;
}

// reminder, we DO NOT set dirty our output (_true_destination, _false_destination) 
void DscDag::DagNodeCondition::MarkDirty()
{
	if (false == _dirty)
	{
		_dirty = true;
		_dag_collection.AddDirtyConditionNode(this);
	}
	return;
}

const bool DscDag::DagNodeCondition::GetHasNoLinks() const
{
	return ((nullptr == _condition) &&
		(nullptr == _true_source) &&
		(nullptr == _true_destination) &&
		(nullptr == _false_source) &&
		(nullptr == _false_destination)
		);
}

void DscDag::DagNodeCondition::SetIndexInput(const int32 in_index, NodeToken in_nodeID)
{
	switch (in_index)
	{
	default:
		DSC_ASSERT_ALWAYS("invalid switch case");
		break;
	case 0:
		_condition = in_nodeID;
		break;
	}
	return;
}

void DscDag::DagNodeCondition::AddInput(NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	DSC_ASSERT((in_nodeID == _true_source) || (in_nodeID == _false_source), "invalid param");
	_input.insert(in_nodeID);
}

void DscDag::DagNodeCondition::RemoveInput(NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	DSC_ASSERT((in_nodeID == _true_source) || (in_nodeID == _false_source), "invalid param");
	_input.erase(in_nodeID);
}

void DscDag::DagNodeCondition::UnlinkInputs()
{
	for (const auto& item : _input)
	{
		if (nullptr != item)
		{
			item->RemoveOutput(this);
		}
	}
	_input.clear();
}

const std::type_info& DscDag::DagNodeCondition::GetTypeInfo() const
{
	return typeid(bool);
}

#if defined(_DEBUG)
const std::string DscDag::DagNodeCondition::DebugPrint(const int32 in_depth) const
{
	std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

	result += "Condition:\"";
	result += _debug_name;
	result += "\" dirty:" + std::to_string(_dirty);
	result += " condition_true:" + std::to_string(_condition_true);
	result += " condition_false:" + std::to_string(_condition_false);
	result += "\n";

	if (nullptr != _condition)
	{
		result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
		result += "condition:\n";
		result += _condition->DebugPrint(in_depth + 2);
	}

	if (nullptr != _true_source)
	{
		result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
		result += "true_source:\n";
		result += _true_source->DebugPrint(in_depth + 2);
	}

	if (nullptr != _true_destination)
	{
		result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
		result += "true_destination:\n";
		result += _true_destination->DebugPrint(in_depth + 2);
	}

	if (nullptr != _false_source)
	{
		result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
		result += "false_source:\n";
		result += _false_source->DebugPrint(in_depth + 2);
	}

	if (nullptr != _false_destination)
	{
		result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
		result += "false_destination:\n";
		result += _false_destination->DebugPrint(in_depth + 2);
	}

	if (0 < _input.size())
	{
		result = DscCommon::DebugPrint::TabDepth(in_depth + 1);
		result += "input:\n";
		for (NodeToken item : _input)
		{
			if (nullptr != item)
			{
				result += item->DebugPrint(in_depth + 2);
			}
		}
	}

	return result;
}
#endif //#if defined(_DEBUG)