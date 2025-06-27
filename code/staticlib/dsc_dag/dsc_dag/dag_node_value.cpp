#include "dag_node_value.h"
#include <dsc_common/vector_int2.h>
#include <dsc_common/vector_float4.h>

namespace
{
	const bool ValueFoundToBeEquavalent(const std::any& in_lhs_value, const std::any& in_rhs_value)
	{
		if ((false == in_lhs_value.has_value()) || (false == in_rhs_value.has_value()))
		{
			return false;
		}
		if (in_lhs_value.type() == in_rhs_value.type())
		{
			if (in_rhs_value.type() == typeid(bool))
			{
				if (std::any_cast<bool>(in_lhs_value) == std::any_cast<bool>(in_rhs_value))
				{
					return true;
				}
			}
			else if (in_rhs_value.type() == typeid(char))
			{
				if (std::any_cast<char>(in_lhs_value) == std::any_cast<char>(in_rhs_value))
				{
					return true;
				}
			}
			else if (in_rhs_value.type() == typeid(double))
			{
				if (std::any_cast<double>(in_lhs_value) == std::any_cast<double>(in_rhs_value))
				{
					return true;
				}
			}
			else if (in_rhs_value.type() == typeid(float))
			{
				if (std::any_cast<float>(in_lhs_value) == std::any_cast<float>(in_rhs_value))
				{
					return true;
				}
			}
			else if (in_rhs_value.type() == typeid(int))
			{
				if (std::any_cast<int>(in_lhs_value) == std::any_cast<int>(in_rhs_value))
				{
					return true;
				}
			}
			else if (in_rhs_value.type() == typeid(std::string))
			{
				if (std::any_cast<std::string>(in_lhs_value) == std::any_cast<std::string>(in_rhs_value))
				{
					return true;
				}
			}
			else if (in_rhs_value.type() == typeid(DscCommon::VectorInt2))
			{
				if (std::any_cast<DscCommon::VectorInt2>(in_lhs_value) == std::any_cast<DscCommon::VectorInt2>(in_rhs_value))
				{
					return true;
				}
			}
			else if (in_rhs_value.type() == typeid(DscCommon::VectorFloat4))
			{
				if (std::any_cast<DscCommon::VectorFloat4>(in_lhs_value) == std::any_cast<DscCommon::VectorFloat4>(in_rhs_value))
				{
					return true;
				}
			}
		}
		return false;
	}

	const bool ValueFoundToNotBeZero(const std::any& in_value)
	{
		if (false == in_value.has_value())
		{
			return false;
		}
		if (in_value.type() == typeid(bool))
		{
			if (true == std::any_cast<bool>(in_value))
			{
				return true;
			}
		}
		else if (in_value.type() == typeid(int32))
		{
			if (0 != std::any_cast<int32>(in_value))
			{
				return true;
			}
		}
		else if (in_value.type() == typeid(float))
		{
			if (0.0f != std::any_cast<float>(in_value))
			{
				return true;
			}
		}

		return false;
	}

}// namespace

DscDag::DagNodeValue::DagNodeValue(const std::any& in_value, const TValueChangeCondition in_change_condition DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name))
	: IDagNode(DSC_DEBUG_ONLY(in_debug_name))
	, _change_condition(in_change_condition)
	, _value(in_value)
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
		// bail out of SetValue without marking diry if value found to be equivalent
		if (true == ValueFoundToBeEquavalent(_value, in_value))
		{
			return;
		}
		set_dirty = true;
		break;
	case TValueChangeCondition::TNotZero:
		if (true == ValueFoundToNotBeZero(in_value))
		{
			set_dirty = true;
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

#if defined(_DEBUG)
const std::string DscDag::DagNodeValue::DebugPrint(const int32 in_depth) const
{
	std::string result = {};
	for (int32 index = 0; index < in_depth; ++index)
	{
		result += "    ";
	}

	result += "Value:\"";
	result += _debug_name;
	result += "\"";
	if (_value.has_value())
	{
		result += " type:";
		result += _value.type().name();
		if (_value.type() == typeid(bool))
		{
			result += " value:" + std::to_string(std::any_cast<bool>(_value));
		}
		else if (_value.type() == typeid(int32))
		{
			result += " value:" + std::to_string(std::any_cast<int32>(_value));
		}
		else if (_value.type() == typeid(float))
		{
			result += " value:" + std::to_string(std::any_cast<float>(_value));
		}
		else if (_value.type() == typeid(DscCommon::VectorInt2))
		{
			DscCommon::VectorInt2 value = std::any_cast<DscCommon::VectorInt2>(_value);
			result += " value:[" + std::to_string(value.GetX());
			result += ", " + std::to_string(value.GetY());
			result += "]";
		}
		else if (_value.type() == typeid(DscCommon::VectorFloat4))
		{
			DscCommon::VectorFloat4 value = std::any_cast<DscCommon::VectorFloat4>(_value);
			result += " value:[" + std::to_string(value.GetX());
			result += ", " + std::to_string(value.GetY());
			result += ", " + std::to_string(value.GetZ());
			result += ", " + std::to_string(value.GetW());
			result += "]";
		}
	}
	result += "\n";

	return result;
}
#endif //#if defined(_DEBUG)
