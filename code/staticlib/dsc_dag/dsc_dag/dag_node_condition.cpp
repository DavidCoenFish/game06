#include "dag_node_condition.h"
#include "dag_collection.h"

namespace
{
}

DscDag::DagNodeCondition::DagNodeCondition(
	DagCollection& in_dag_collection,
	NodeToken in_true_destination,
	NodeToken in_false_destination
	DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name)
)
	: IDagNode(DSC_DEBUG_ONLY(in_debug_name))
	, _dag_collection(in_dag_collection)
	, _true_destination(in_true_destination)
	, _false_destination(in_false_destination)
{
	//nop
}

const std::any& DscDag::DagNodeCondition::GetValue()
{
	_dirty = false;

	if (nullptr == _condition)
	{
		_value = {};
		return _value;
	}

	if (true == DagCollection::GetValueType<bool>(_condition))
	{
		_value = true;
		std::any true_value = {};
		if (nullptr != _true_source)
		{
			true_value = _true_source->GetValue();
		}
		if (nullptr != _true_destination)
		{
			_true_destination->SetValue(true_value);
		}
	}
	else
	{
		_value = false;
		std::any false_value = {};
		if (nullptr != _false_source)
		{
			false_value = _false_source->GetValue();
		}
		if (nullptr != _false_destination)
		{
			_false_destination->SetValue(false_value);
		}
	}

	return _value;
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
		break;
	case 0:
		_condition = in_nodeID;
		break;
	case 1:
		_true_source = in_nodeID;
		break;
	case 2:
		_false_source = in_nodeID;
		break;
	}
	return;
}

#if defined(_DEBUG)
const std::string DscDag::DagNodeCondition::DebugPrint(const int32 in_depth) const
{
	std::string result = {};
	for (int32 index = 0; index < in_depth; ++index)
	{
		result += "    ";
	}

	result += "Condition:\"";
	result += _debug_name;
	result += "\" dirty:" + std::to_string(_dirty);
	result += "\n";

	if (nullptr != _condition)
	{
		for (int32 index = 0; index < in_depth + 1; ++index)
		{
			result += "    ";
		}
		result += "_condition:\n";
		result += _condition->DebugPrint(in_depth + 2);
	}

	if (nullptr != _true_source)
	{
		for (int32 index = 0; index < in_depth + 1; ++index)
		{
			result += "    ";
		}
		result += "_true_source:\n";
		result += _true_source->DebugPrint(in_depth + 2);
	}

	if (nullptr != _false_source)
	{
		for (int32 index = 0; index < in_depth + 1; ++index)
		{
			result += "    ";
		}
		result += "_false_source:\n";
		result += _false_source->DebugPrint(in_depth + 2);
	}

	return result;
}
#endif //#if defined(_DEBUG)