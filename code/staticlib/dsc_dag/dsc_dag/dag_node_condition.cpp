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
	_dag_collection.AddDirtyConditionNode(this);
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

DscDag::NodeToken DscDag::DagNodeCondition::GetIndexInput(const int32 in_index) const
{
	switch (in_index)
	{
	default:
		break;
	case 0:
		return _condition;
	case 1:
		return _true_source;
	case 2:
		return _false_source;
	}
	return nullptr;
}