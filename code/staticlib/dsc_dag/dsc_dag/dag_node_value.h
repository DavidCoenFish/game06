#pragma once
#include "dsc_dag.h"
#include "i_dag_node.h"
#include "dag_node_calculate.h"
#include <dsc_common\dsc_common.h>
#include <dsc_common\debug_print.h>

namespace DscDag
{
	// note: these are in structs as to make them easy to pass into template param
	// however, we are not using them as template param as for dynamic casting to work, you need to know all the template param, and we just use type for methods like get value...
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
	struct CallbackNotZero {
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
			const TCallbackOnSet& in_callback_on_set_value_or_null = CallbackOnValueChange<IN_TYPE>::Function
			)
			: _value(in_value)
			, _callback_on_set(in_callback_on_set_value_or_null)
		{
			// Nop
		}

		const IN_TYPE& GetValue() const
		{
			return _value;
		}

		IN_TYPE& GetValueNonConst(const bool in_set_dirty)
		{
			if (true == in_set_dirty)
			{
				SetDirty();
			}
			return _value;
		}

		void SetValue(const IN_TYPE& in_value)
		{
			bool set_dirty = false;
			bool bail = false;

			if (nullptr != _callback_on_set)
			{
				_callback_on_set(set_dirty, bail, _value, in_value);
			}

			if (true == bail)
			{
				return;
			}

			_value = in_value;
			if (true == set_dirty)
			{
				SetDirty();
			}
		}

	private:
		void SetDirty()
		{
			for (auto& item : _output)
			{
				item->MarkDirty();
			}
		}

		virtual void AddOutput(NodeToken in_nodeID) override
		{
			//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "AddOutput:%p %p\n", this, in_nodeID);

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
				if (0 != _output.erase(in_nodeID))
				{
					//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "RemoveOutput:%p %p\n", this, in_nodeID);
					in_nodeID->MarkDirty();
				}
			}
		}

		virtual const bool GetHasNoLinks() const override
		{
			return (0 == _output.size());
		}

#if defined(_DEBUG)
		virtual const std::type_info& DebugGetTypeInfo() const override
		{
			return typeid(IN_TYPE);
		}

		virtual const std::string DebugPrintRecurseInputs(const int32 in_depth = 0) const override
		{
			std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

			result += "Value:\"";
			result += DebugGetNodeName();
			result += "\"";
			result += " type:";
			result += typeid(IN_TYPE).name();
			if (nullptr != _s_debug_print_value)
			{
				result += " value:";
				result += _s_debug_print_value(_value);
			}
			result += "\n";

			return result;
		}

		virtual const std::string DebugPrintRecurseOutputs(const int32 in_depth = 0) const override
		{
			std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

			result += "Value:\"";
			result += DebugGetNodeName();
			result += "\"";
			result += " type:";
			result += typeid(IN_TYPE).name();
			if (nullptr != _s_debug_print_value)
			{
				result += " value:";
				result += _s_debug_print_value(_value);
			}
			result += "\n";

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
public:
	typedef std::function<std::string(const IN_TYPE&)> TDebugPrintValue;
	static inline TDebugPrintValue _s_debug_print_value = {};
#endif //#if defined(_DEBUG)

	private:
		IN_TYPE _value = {};
		std::set<NodeToken> _output = {};
		TCallbackOnSet _callback_on_set = {};
	};

} //DscDag
