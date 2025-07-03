#pragma once
#include "dsc_dag.h"
#include "i_dag_node.h"
#include "dag_enum.h"
#include "dag_node_calculate.h"
#include <dsc_common\dsc_common.h>
#include <dsc_common\debug_print.h>
#include <dsc_common\vector_2.h>
#include <dsc_common\vector_4.h>

namespace DscDag
{
	template <typename IN_TYPE>
	struct CallbackOnSetValue {
		static void Function(bool& out_dirty, bool& out_bail, const IN_TYPE&, const IN_TYPE&)
		{
			out_dirty = true;
			out_bail = false;
		}
	};

	template <typename IN_TYPE>
	struct CallbackNever {
		static void Function(bool& out_dirty, bool& out_bail, const IN_TYPE&, const IN_TYPE&)
		{
			out_dirty = false;
			out_bail = false;
		}
	};

	template <typename IN_TYPE>
	struct CallbackOnValueChange {
		static void Function(bool& out_dirty, bool& out_bail, const IN_TYPE& in_lhs, const IN_TYPE& in_rhs)
		{
			out_dirty = true;
			out_bail = (in_lhs == in_rhs);
		}
	};

	template <typename IN_TYPE>
	struct CallbackNoZero {
		static void Function(bool& out_dirty, bool& out_bail, const IN_TYPE&, const IN_TYPE& in_rhs)
		{
			out_dirty = 0 != in_rhs;
			out_bail = false;
		}
	};

	// interest in a templated version as you can't compare std::any without knowing the type
	//template <typename IN_TYPE, typename IN_CALLBACK = CallbackOnValueChange<IN_TYPE> DSC_DEBUG_ONLY(DSC_COMMA typename IN_DEBUG_PRINT = DscCommon::DebugPrintNone<IN_TYPE>)>
	template <typename IN_TYPE>
	class DagNodeValue : public IDagNode
	{
	public:
		typedef std::function<void(bool& out_dirty, bool& out_bail, const IN_TYPE&, const IN_TYPE& in_rhs)> TCallbackOnSet;

		DagNodeValue(const IN_TYPE& in_value, 
			const TCallbackOnSet& in_callback_on_set_value = CallbackOnValueChange<IN_TYPE>::Function
			DSC_DEBUG_ONLY(DSC_COMMA const std::string & in_debug_name = ""))
			: IDagNode(DSC_DEBUG_ONLY(in_debug_name))
			, _value(in_value)
			, _callback_on_set(in_callback_on_set_value)
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
			bool bail = false;
			//IN_CALLBACK::Function(set_dirty, bail, _value, in_value);
			_callback_on_set(set_dirty, bail, _value, in_value);

			if (true == bail)
			{
				return;
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
			//result += " value:" + IN_DEBUG_PRINT::Function(_value);
			result += "\n";

			return result;
		}
#endif //#if defined(_DEBUG)

	private:
		IN_TYPE _value = {};
		std::set<NodeToken> _output = {};
		TCallbackOnSet _callback_on_set = {};
	};

} //DscDag
