#include "dag_node_calculate.h"
#include <dsc_common/vector_int2.h>
#include <dsc_common/vector_float4.h>

DscDag::DagNodeCalculate::DagNodeCalculate(const TCalculateFunction& in_calculateFunction DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name))
	: IDagNode(DSC_DEBUG_ONLY(in_debug_name))
	, _calculateFunction(in_calculateFunction)
{
	// nop
}

void DscDag::DagNodeCalculate::MarkDirty()
{
	if (true == _dirty)
	{
		return;
	}
	_dirty = true;
	for (auto& item : _output)
	{
		item->MarkDirty();
	}
}

void DscDag::DagNodeCalculate::AddOutput(NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	if (nullptr != in_nodeID)
	{
		in_nodeID->MarkDirty();
	}
	_output.insert(in_nodeID);
}

void DscDag::DagNodeCalculate::RemoveOutput(NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	if (nullptr != in_nodeID)
	{
		in_nodeID->MarkDirty();
	}
	_output.erase(in_nodeID);
}

void DscDag::DagNodeCalculate::SetIndexInput(const int32 in_index, NodeToken in_nodeID)
{
	DSC_ASSERT(0 <= in_index, "invalid param");
	// null in_nodeID allowed
	if (static_cast<int32>(_indexInput.size()) <= in_index)
	{
		_indexInput.resize(in_index + 1);
	}
	_indexInput[in_index] = in_nodeID;
	MarkDirty();
}

//DscDag::NodeToken DscDag::DagNodeCalculate::GetIndexInput(const int32 in_index) const
//{
//	DSC_ASSERT(0 <= in_index, "invalid param");
//	// null in_nodeID allowed
//	if ((0 <= in_index) && (in_index < static_cast<int32>(_indexInput.size())))
//	{
//		return _indexInput[in_index];
//	}
//	return nullptr;
//}

void DscDag::DagNodeCalculate::AddInput(NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	_input.insert(in_nodeID);
	MarkDirty();
}

void DscDag::DagNodeCalculate::RemoveInput(NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	_input.erase(in_nodeID);
	MarkDirty();
}

const std::any& DscDag::DagNodeCalculate::GetValue()
{
	if (true == _dirty)
	{
		_dirty = false;

		// so, currently we could have input that is not used in the calculate, and would keep it's dirty flag. 
		// if it was dirtied again, the mark dirty would be skipped as it is already dirty
		// so, on each call to calculate, explicity GetValue on all inputs to flush the dirty state
		for (auto& item : _input)
		{
			if (nullptr != item)
			{
				item->GetValue();
			}
		}
		for (auto& item : _indexInput)
		{
			if (nullptr != item)
			{
				item->GetValue();
			}
		}

		_calculateFunction(_value, _input, _indexInput);
	}
	return _value;
}


const bool DscDag::DagNodeCalculate::GetHasNoLinks() const
{
	if (0 != _output.size())
	{
		return false;
	}
	if (0 != _input.size())
	{
		return false;
	}
	for (const auto& item : _indexInput)
	{
		if (nullptr != item)
		{
			return false;
		}
	}
	return true;
}

#if defined(_DEBUG)
const std::string DscDag::DagNodeCalculate::DebugPrint(const int32 in_depth) const
{
	std::string result = {};
	for (int32 index = 0; index < in_depth; ++index)
	{
		result += "    ";
	}

	result += "Calculate:\"";
	result += _debug_name;
	result += "\" dirty:" + std::to_string(_dirty);

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
	if (0 < _input.size())
	{
		for (int32 index = 0; index < in_depth + 1; ++index)
		{
			result += "    ";
		}
		result += "_input:\n";

		for (NodeToken item : _input)
		{
			if (nullptr != item)
			{
				result += item->DebugPrint(in_depth + 2);
			}
		}
	}
	if (0 < _indexInput.size())
	{
		for (int32 index = 0; index < in_depth + 1; ++index)
		{
			result += "    ";
		}
		result += "_indexInput:\n";

		for (NodeToken item : _indexInput)
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
