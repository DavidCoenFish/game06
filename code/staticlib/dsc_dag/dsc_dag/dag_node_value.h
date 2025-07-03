#pragma once
#include "dsc_dag.h"
#include "i_dag_node.h"
#include "dag_enum.h"
#include "dag_node_calculate.h"
#include <dsc_common\dsc_common.h>
#include <dsc_common\debug_print.h>

namespace DscDag
{
	// interest in a templated version as you can't compare std::any without knowing the type
	template <typename IN_TYPE>
	class DagNodeValue : public IDagNode
	{
	public:
		DagNodeValue(const IN_TYPE& in_value, const TValueChangeCondition in_change_condition = TValueChangeCondition::TOnValueChange DSC_DEBUG_ONLY(DSC_COMMA const std::string & in_debug_name = ""))
			: IDagNode(DSC_DEBUG_ONLY(in_debug_name))
			, _change_condition(in_change_condition)
			, _value(in_value)
		{
			// Nop
		}

		const IN_TYPE& GetValue() const
		{
			return _value;
		}

		void SetValue(const IN_TYPE& in_value)
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
				// bail out of SetValue without marking dirty if value found to be equivalent
				if (_value == in_value)
				{
					return;
				}
				set_dirty = true;
				break;
			case TValueChangeCondition::TNotZero:
				if (in_value != 0)
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

	private:
		virtual void AddOutput(NodeToken in_nodeID) override
		{
			DSC_ASSERT(nullptr != in_nodeID, "invalid param");
			if (nullptr != in_nodeID)
			{
				_output.insert(in_nodeID);
				in_nodeID->MarkDirty();
			}
		}

		virtual void RemoveOutput(NodeToken in_nodeID) override
		{
			DSC_ASSERT(nullptr != in_nodeID, "invalid param");
			if (nullptr != in_nodeID)
			{
				_output.erase(in_nodeID);
				in_nodeID->MarkDirty();
			}
		}

		virtual const bool GetHasNoLinks() const override
		{
			return (0 == _output.size());
		}

		virtual const std::type_info& GetTypeInfo() const override
		{
			return typeid(IN_TYPE);
		}

		// for DagCondition, wanted to be able to assign output of one node to another node
		virtual void SetFromNode(IDagNode* const in_node) override
		{
			// break a cyclic dependency, would normally call DagCollection::GetValueType, but it needs to have our definition
			auto value_node = dynamic_cast<DagNodeValue<IN_TYPE>*>(in_node);
			if (nullptr != value_node)
			{
				SetValue(value_node->GetValue());
				return;
			}
			auto calculate_node = dynamic_cast<DagNodeCalculate<IN_TYPE>*>(in_node);
			if (nullptr != calculate_node)
			{
				SetValue(calculate_node->GetValue());
				return;
			}

			DSC_ASSERT_ALWAYS("invalid code path");
			return;
		}

#if defined(_DEBUG)
		virtual const std::string DebugPrint(const int32 in_depth = 0) const override
		{
			std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

			result += "Value:\"";
			result += _debug_name;
			result += "\"";
			result += " type:";
			result += typeid(IN_TYPE).name();
			result += " value:" + DscCommon::DebugPrint::PrintType(_value);
			result += "\n";

			return result;
		}
#endif //#if defined(_DEBUG)

	private:
		TValueChangeCondition _change_condition = {};
		IN_TYPE _value = {};
		std::set<NodeToken> _output = {};

	};

} //DscDag
