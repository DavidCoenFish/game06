#include <dsc_dag/dag_node_value.h>

namespace
{
}

DscDag::DagNodeValue::DagNodeValue(const std::any& in_value, const TValueChangeCondition in_change_condition)
	: _value(in_value)
	, _change_condition(in_change_condition)
{
	// Nop
}

void DscDag::DagNodeValue::AddOutput(NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	if (nullptr != in_nodeID)
	{
		_output.insert(in_nodeID);
		in_nodeID->MarkDirty();
	}
}

void DscDag::DagNodeValue::RemoveOutput(NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	if (nullptr != in_nodeID)
	{
		_output.erase(in_nodeID);
		in_nodeID->MarkDirty();
	}
}

void DscDag::DagNodeValue::SetValue(const std::any& in_value)
{
	bool set_dirty = false;

	switch (_change_condition)
	{
	default:
		break;
	case TValueChangeCondition::TOnSet:
		set_dirty = true;
		break;
	case TValueChangeCondition::TNever:
		break;
	case TValueChangeCondition::TOnValueChange:
		// bail out if value is already equal
		if (_value.has_value() && in_value.has_value())
		{
			if (_value.type() == in_value.type())
			{
				if (in_value.type() == typeid(bool))
				{
					if (std::any_cast<bool>(_value) == std::any_cast<bool>(in_value))
					{
						return;
					}
				}
				else if (in_value.type() == typeid(char))
				{
					if (std::any_cast<char>(_value) == std::any_cast<char>(in_value))
					{
						return;
					}
				}
				else if (in_value.type() == typeid(double))
				{
					if (std::any_cast<double>(_value) == std::any_cast<double>(in_value))
					{
						return;
					}
				}
				else if (in_value.type() == typeid(float))
				{
					if (std::any_cast<float>(_value) == std::any_cast<float>(in_value))
					{
						return;
					}
				}
				else if (in_value.type() == typeid(int))
				{
					if (std::any_cast<int>(_value) == std::any_cast<int>(in_value))
					{
						return;
					}
				}
				else if (in_value.type() == typeid(std::string))
				{
					if (std::any_cast<std::string>(_value) == std::any_cast<std::string>(in_value))
					{
						return;
					}
				}

			}
		}
		break;

	}

	_value = in_value;
	if (true == set_dirty)
	{
		for (auto& item : _output)
		{
			item->MarkDirty();
		}
	}
}

const std::any& DscDag::DagNodeValue::GetValue()
{
	return _value;
}

const bool DscDag::DagNodeValue::GetHasNoLinks() const
{
	return (0 == _output.size());
}


